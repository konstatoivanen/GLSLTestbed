#include "PrecompiledHeader.h"
#include "LightsManager.h"
#include "Rendering/Graphics.h"
#include "Utilities/HashCache.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

namespace PK::Rendering
{
	struct ShadowmapContext
	{
		ShadowmapData* data;
		uint index;
	};

	static void InitializeShadowmapData(AssetDatabase* assetDatabase, ShadowmapData* data)
	{
		data->ShaderRenderCube = assetDatabase->Find<Shader>("SH_VS_ShadowmapCube");
		data->ShaderBlurCube = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurCube");

		auto descriptor = RenderTextureDescriptor();
		descriptor.dimension = GL_TEXTURE_CUBE_MAP_ARRAY;
		descriptor.resolution = { 256, 256, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
		data->ShadowmapCube = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D_ARRAY;
		descriptor.resolution = { ShadowmapData::TileSize, ShadowmapData::TileSize, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_NONE;
		data->ShadowmapOctahedron = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D;
		descriptor.colorFormats = { GL_RG32F };
		descriptor.resolution = { ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, 0 };

		data->ShadowmapAtlas = CreateRef<RenderTexture>(descriptor);
	}

	static float4x4 GetCubefaceMatrix(const float3& position, const float3& s, const float3& f, const float3& u, float znear, float zfar)
	{
		const float3 proj = float3((zfar + znear) / (zfar - znear), -(2.0f * zfar * znear) / (zfar - znear), glm::dot(f, position));
		return float4x4(
			float4(s.x, u.x, proj.x * -f.x, -f.x), 
			float4(s.y, u.y, proj.x * -f.y, -f.y), 
			float4(s.z, u.z, proj.x * -f.z, -f.z),
			float4(-glm::dot(s, position), -glm::dot(u, position), proj.x * proj.z + proj.y, proj.z));
	}	

	// @TODO implement spot lights
	static void RenderShadowmaps(ShadowmapData* data, ECS::EntityDatabase* entityDb, BufferView<uint>& lightIndices, ShaderPropertyBlock& properties)
	{
		auto shaderRenderCube = data->ShaderRenderCube.lock().get();
		auto shaderBlurCube = data->ShaderBlurCube.lock().get();

		auto lightCount = glm::min(ShadowmapData::TileCountPerAxis * ShadowmapData::TileCountPerAxis, (uint)lightIndices.count);
		auto batchCount = (uint)ceil((float)lightCount / ShadowmapData::BatchSize);
		const auto znear = 0.01f;

		float4 viewports[ShadowmapData::BatchSize]{};

		for (auto i = 0; i < ShadowmapData::BatchSize; ++i)
		{
			viewports[i].zw = float2(ShadowmapData::TileSize, ShadowmapData::TileSize);
		}

		float3 lightPositions[ShadowmapData::BatchSize]{};

		Culling::OnVisibleItem onvisible = [](ECS::EntityDatabase* entityDb, ECS::EGID egid, float depth, void* context)
		{
			auto ctx = reinterpret_cast<ShadowmapContext*>(context);
			auto renderable = entityDb->Query<ECS::EntityViews::MeshRenderable>(egid);
			Batching::QueueDraw(&ctx->data->Batches, renderable->mesh->sharedMesh, { &renderable->transform->localToWorld, depth, ctx->index });
		};

		for (uint batchIdx = 0; batchIdx < batchCount; ++batchIdx)
		{
			auto baseIdx = batchIdx * ShadowmapData::BatchSize;
			auto batchSize = glm::min((uint)(lightCount - baseIdx), ShadowmapData::BatchSize);
			auto zfar = 0.0f;

			for (uint i = 0; i < batchSize; ++i)
			{
				auto* light = entityDb->Query<ECS::EntityViews::PointLightRenderable>(ECS::EGID(lightIndices[baseIdx + i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
				lightPositions[i] = light->transform->position;
				
				if (light->pointLight->radius > zfar)
				{
					zfar = light->pointLight->radius;
				}
			}

			Batching::ResetCollection(&data->Batches);

			for (uint i = 0; i < batchSize; ++i)
			{
				auto lightIndex = baseIdx + i;
				auto baseKey = ((uint)i << 16u) | (lightIndex & 0xFFFF);

				// @TODO optimize later
				float4x4 facematrices[6] =
				{
					GetCubefaceMatrix(lightPositions[i], CG_FLOAT3_FORWARD, CG_FLOAT3_RIGHT, CG_FLOAT3_UP, znear, zfar),
					GetCubefaceMatrix(lightPositions[i], CG_FLOAT3_BACKWARD, CG_FLOAT3_LEFT, CG_FLOAT3_UP, znear, zfar),
					GetCubefaceMatrix(lightPositions[i], CG_FLOAT3_RIGHT, CG_FLOAT3_DOWN, CG_FLOAT3_FORWARD, znear, zfar),
					GetCubefaceMatrix(lightPositions[i], CG_FLOAT3_LEFT, CG_FLOAT3_UP, CG_FLOAT3_FORWARD, znear, zfar),
					GetCubefaceMatrix(lightPositions[i], CG_FLOAT3_LEFT, CG_FLOAT3_BACKWARD, CG_FLOAT3_UP, znear, zfar),
					GetCubefaceMatrix(lightPositions[i], CG_FLOAT3_RIGHT, CG_FLOAT3_FORWARD, CG_FLOAT3_UP, znear, zfar)
				};

				viewports[i].x = (float)ShadowmapData::TileSize * (lightIndex % ShadowmapData::TileCountPerAxis);
				viewports[i].y = (float)ShadowmapData::TileSize * (lightIndex / ShadowmapData::TileCountPerAxis);

				for (uint face = 0; face < 6; ++face)
				{
					ShadowmapContext ctx = { data, (face << 24u) | baseKey };
					Culling::ExecuteOnVisibleItemsFrustum(entityDb, facematrices[face], (ushort)(ECS::Components::RenderHandleFlags::Renderer | ECS::Components::RenderHandleFlags::ShadowCaster), onvisible, &ctx);
				}
			}

			Batching::UpdateBuffers(&data->Batches);

			GraphicsAPI::SetRenderTarget(data->ShadowmapCube.get());
			GraphicsAPI::Clear(float4(zfar, zfar * zfar, 0, 0), 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Batching::DrawBatches(&data->Batches, 0, shaderRenderCube, properties);
			
			properties.SetKeywords({ StringHashID::StringToID("BLUR_PASS0") });
			GraphicsAPI::BlitInstanced(0, batchSize, data->ShadowmapCube->GetColorBufferPtr(0), data->ShadowmapOctahedron.get(), shaderBlurCube, properties);

			GraphicsAPI::SetRenderTarget(data->ShadowmapAtlas.get(), 0, viewports, batchSize);
			properties.SetKeywords({ StringHashID::StringToID("BLUR_PASS1") });
			GraphicsAPI::BlitInstanced(baseIdx, batchSize, data->ShadowmapOctahedron->GetColorBufferPtr(0), data->ShadowmapAtlas.get(), shaderBlurCube, properties);
		}

		GraphicsAPI::SetGlobalTexture(StringHashID::StringToID("pk_ShadowmapAtlas"), data->ShadowmapAtlas->GetColorBufferPtr(0)->GetGraphicsID());
	}


	LightsManager::LightsManager(AssetDatabase* assetDatabase)
	{
		m_passKeywords[0] = StringHashID::StringToID("PASS_CLUSTERS");
		m_passKeywords[1] = StringHashID::StringToID("PASS_DISPATCH");
	
		m_computeLightAssignment = assetDatabase->Find<Shader>("CS_ClusteredLightAssignment");
		m_computeDepthReset = assetDatabase->Find<Shader>("CS_ClusteredDepthReset");
		m_computeDepthTiles = assetDatabase->Find<Shader>("CS_ClusteredDepthMinMax");
		m_computeCullClusters = assetDatabase->Find<Shader>("CS_ClusteredCullClusters");
		m_debugVisualize = assetDatabase->Find<Shader>("SH_VS_ClusterDebug");
	
		InitializeShadowmapData(assetDatabase, &m_shadowmapData);

		m_depthTiles = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT2, "DEPTHMINMAX"},
		}), ClusterCount, true, GL_NONE);
	
		m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4, "COLOR"},
			{CG_TYPE::FLOAT4, "DIRECTION"},
			{CG_TYPE::UINT, "SHADOWMAP_INDEX"},
			{CG_TYPE::UINT, "SHADOWMAP_PROJ_INDEX"},
			{CG_TYPE::UINT, "LIGHT_COOKIE_INDEX"},
			{CG_TYPE::UINT, "LIGHT_TYPE"},
		}), 32, false, GL_DYNAMIC_DRAW);
	
		m_VisibleClusterList = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "INDEX"},
		}), ClusterCount, true, GL_NONE);
	
		m_clusterDispatchInfo = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "DISPATCH_ARGUMENTS", 5}
		}), 1, true, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
	
		m_globalLightsList = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "INDEX" }
		}), ClusterCount * MaxLightsPerTile, true, GL_NONE);
	
		m_lightTiles = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "INDEX"},
		}), ClusterCount, true, GL_NONE);
	}
	
	void LightsManager::Preprocess(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution, float znear, float zfar)
	{
		m_lightsBuffer->ValidateSize((uint)visibleLights.count + 1);
		
		auto buffer = m_lightsBuffer->BeginMapBufferRange<Structs::PKRawLight>(0, visibleLights.count + 1);
	
		for (size_t i = 0; i < visibleLights.count; ++i)
		{
			auto* view = entityDb->Query<ECS::EntityViews::PointLightRenderable>(ECS::EGID(visibleLights[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
			buffer[i] = { view->pointLight->color, float4(view->transform->position, view->pointLight->radius), (uint)i, 0u, 0u, 0u };
		}
	
		buffer[visibleLights.count] = { CG_COLOR_CLEAR, CG_FLOAT4_ZERO, 0u, 0u, 0u, 0u };
		m_lightsBuffer->EndMapBuffer();

		float frustuminfo[5] = 
		{ 
			(float)GridSizeZ / glm::log2(zfar / znear),
			-(float)GridSizeZ * glm::log2(znear) / log2(zfar / znear), 
			std::ceilf(resolution.x / (float)GridSizeX), 
			znear,
			zfar / znear
		};
	
		auto hashCache = HashCache::Get();
	
		GraphicsAPI::SetGlobalInt(hashCache->pk_LightCount, (int)visibleLights.count);
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_Lights, m_lightsBuffer->GetGraphicsID());
		GraphicsAPI::SetGlobalFloat(hashCache->pk_ClusterFrustumInfo, frustuminfo, 5);
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_GlobalLightsList, m_globalLightsList->GetGraphicsID());
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_LightTiles, m_lightTiles->GetGraphicsID());
		m_properties.SetComputeBuffer(hashCache->pk_FDepthRanges, m_depthTiles->GetGraphicsID());
		m_properties.SetComputeBuffer(hashCache->pk_ClusterDispatchInfo, m_clusterDispatchInfo->GetGraphicsID());
		m_properties.SetComputeBuffer(hashCache->pk_VisibleClusters, m_VisibleClusterList->GetGraphicsID());
	
		uint zero[5] = { 0, 0, 0, 0, 0 };
		m_clusterDispatchInfo->SubmitData(&zero, 0, CG_TYPE_SIZE_INT * 5);

		RenderShadowmaps(&m_shadowmapData, entityDb, visibleLights, m_properties);
	}
	
	void LightsManager::UpdateLightTiles(const uint2& resolution)
	{
		auto depthCountX = (uint)std::ceilf(resolution.x / 16.0f);
		auto depthCountY = (uint)std::ceilf(resolution.y / 16.0f);
	
		GraphicsAPI::DispatchCompute(m_computeDepthReset.lock().get(), { 1,1, GridSizeZ / 4 }, m_properties);
		GraphicsAPI::DispatchCompute(m_computeDepthTiles.lock().get(), { depthCountX, depthCountY, 1 }, m_properties);
		
		m_properties.SetKeywords({ m_passKeywords[0] });
		GraphicsAPI::DispatchCompute(m_computeCullClusters.lock().get(), { GridSizeX, GridSizeY, GridSizeZ }, m_properties);
	
		m_properties.SetKeywords({ m_passKeywords[1] });
		GraphicsAPI::DispatchCompute(m_computeCullClusters.lock().get(), { 1, 1, 1}, m_properties);
		
		GraphicsAPI::DispatchComputeIndirect(m_computeLightAssignment.lock().get(), m_clusterDispatchInfo->GetGraphicsID(), 0, m_properties);
	}
	
	void LightsManager::DrawDebug()
	{
		GraphicsAPI::Blit(m_debugVisualize.lock().get(), m_properties);
	}
}