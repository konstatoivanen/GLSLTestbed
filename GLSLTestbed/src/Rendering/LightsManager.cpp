#include "PrecompiledHeader.h"
#include "LightsManager.h"
#include "Rendering/Graphics.h"
#include "Rendering/Culling.h"
#include "Utilities/HashCache.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

namespace PK::Rendering
{
	static void InitializeShadowmapData(AssetDatabase* assetDatabase, ShadowmapData* data)
	{
		data->ShaderRenderCube = assetDatabase->Find<Shader>("SH_VS_ShadowmapCube");
		data->ShaderBlurCube = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurCube");
		data->ShaderBlitToAtlas = assetDatabase->Find<Shader>("SH_VS_ShadowmapAtlas");

		auto descriptor = RenderTextureDescriptor();
		descriptor.dimension = GL_TEXTURE_CUBE_MAP;
		descriptor.resolution = { 256, 256, 0 };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		data->ShadowmapCube = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D;
		descriptor.resolution = { ShadowmapData::TileSize, ShadowmapData::TileSize, 0 };
		descriptor.colorFormats = { GL_RG32F, GL_RG32F };
		descriptor.depthFormat = GL_NONE;
		descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		data->ShadowmapOctahedron = CreateRef<RenderTexture>(descriptor);

		descriptor.colorFormats = { GL_RG32F };
		descriptor.resolution = { ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, 0 };

		data->ShadowmapAtlas = CreateRef<RenderTexture>(descriptor);
	}

	static void RenderShadowmapPointLight(ShadowmapData* data, ECS::EntityDatabase* entityDb, ECS::EntityViews::PointLightRenderable* pointLight, uint shadowmapIndex, ShaderPropertyBlock& properties)
	{
		auto position = pointLight->transform->position;
		const float zNear = 0.2f;
		const float zFar = pointLight->pointLight->radius;
		auto proj = Functions::GetPerspective(90.0f, 1.0f, zNear, zFar);

		properties.SetFloat4x4(StringHashID::StringToID("pk_ShadowmapMatrix"), proj);
		properties.SetUInt(StringHashID::StringToID("pk_ShadowmapLightIndex"), shadowmapIndex);

		auto shaderRenderCube = data->ShaderRenderCube.lock().get();
		auto shaderBlurCube = data->ShaderBlurCube.lock().get();
		auto shaderBlitToAtlas = data->ShaderBlitToAtlas.lock().get();

		Culling::VisibilityList vis;
		Culling::BuildVisibilityListSphere(entityDb, &vis, position, pointLight->pointLight->radius, (ushort)(ECS::Components::RenderHandleFlags::Renderer | ECS::Components::RenderHandleFlags::ShadowCaster));

		Batching::ResetCollection(&data->Batches);
		
		auto itemIds = vis.list.data();
		for (auto i = 0; i < vis.count; ++i)
		{

			auto renderable = entityDb->Query<ECS::EntityViews::MeshRenderable>(ECS::EGID(itemIds[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
			Batching::QueueDraw(&data->Batches, renderable->mesh->sharedMesh, &renderable->transform->localToWorld, &renderable->transform->worldToLocal);
		}

		Batching::UpdateBuffers(&data->Batches);


		GraphicsAPI::SetRenderTarget(data->ShadowmapCube.get());
		GraphicsAPI::Clear(float4(zFar, zFar * zFar, 0, 0), 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Batching::DrawBatches(&data->Batches, 0, shaderRenderCube, properties);
		
		data->ShadowmapOctahedron->SetDrawTargets({ GL_COLOR_ATTACHMENT0 });
		properties.SetKeywords({ StringHashID::StringToID("BLUR_PASS0") });
		GraphicsAPI::Blit(data->ShadowmapCube->GetColorBufferPtr(0), data->ShadowmapOctahedron.get(), shaderBlurCube, properties);

		data->ShadowmapOctahedron->SetDrawTargets({ GL_COLOR_ATTACHMENT1 });
		properties.SetKeywords({ StringHashID::StringToID("BLUR_PASS1") });
		GraphicsAPI::Blit(data->ShadowmapOctahedron->GetColorBufferPtr(0), data->ShadowmapOctahedron.get(), shaderBlurCube, properties);

		properties.SetUInt(StringHashID::StringToID("pk_ShadowmapIndex"), shadowmapIndex);
		GraphicsAPI::Blit(data->ShadowmapOctahedron->GetColorBufferPtr(1), data->ShadowmapAtlas.get(), shaderBlitToAtlas, properties);

		GraphicsAPI::SetGlobalTexture(StringHashID::StringToID("pk_ShadowmapAtlas"), data->ShadowmapAtlas->GetColorBufferPtr(0)->GetGraphicsID());
	}


	LightsManager::LightsManager(AssetDatabase* assetDatabase)
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
		}), ClusterCount, GL_STATIC_COPY);
	
		m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4, "COLOR"},
			{CG_TYPE::FLOAT4, "DIRECTION"},
			{CG_TYPE::UINT, "SHADOWMAP_INDEX"},
			{CG_TYPE::UINT, "SHADOWMAP_PROJ_INDEX"},
			{CG_TYPE::UINT, "LIGHT_COOKIE_INDEX"},
			{CG_TYPE::UINT, "LIGHT_TYPE"},
		}), 32);
	
		m_VisibleClusterList = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "INDEX"},
		}), ClusterCount, GL_STATIC_COPY);
	
		m_clusterDispatchInfo = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "DISPATCH_ARGUMENTS", 5}
		}), 1);
	
		m_globalLightsList = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "INDEX" }
		}), ClusterCount * MaxLightsPerTile, GL_STATIC_COPY);
	
		m_lightTiles = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT, "INDEX"},
		}), ClusterCount, GL_STATIC_COPY);
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

		if (visibleLights.count > 0)
		{
			for (size_t i = 0; i < visibleLights.count; ++i)
			{
				auto* view = entityDb->Query<ECS::EntityViews::PointLightRenderable>(ECS::EGID(visibleLights[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
				RenderShadowmapPointLight(&m_shadowmapData, entityDb, view, (uint)i, m_properties);
			}
		}
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