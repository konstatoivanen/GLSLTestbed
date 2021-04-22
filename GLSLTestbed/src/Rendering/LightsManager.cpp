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

	static void OnCullVisibleShadowmap(ECS::EntityDatabase* entityDb, ECS::EGID egid, uint clipIndex, float depth, void* context)
	{
		auto ctx = reinterpret_cast<ShadowmapContext*>(context);
		auto renderable = entityDb->Query<ECS::EntityViews::MeshRenderable>(egid);
		auto index = (clipIndex << 24u) | ctx->index;
		Batching::QueueDraw(&ctx->data->Batches, renderable->mesh->sharedMesh, { &renderable->transform->localToWorld, depth, index });
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
	
		m_shadowmapData.ShaderRenderMapCube = assetDatabase->Find<Shader>("SH_WS_ShadowmapCube");
		m_shadowmapData.ShaderRenderMapProj = assetDatabase->Find<Shader>("SH_WS_ShadowmapProj");
		m_shadowmapData.ShaderBlurCube = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurCube");
		m_shadowmapData.ShaderBlurProj = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurProj");

		auto descriptor = RenderTextureDescriptor();
		descriptor.dimension = GL_TEXTURE_CUBE_MAP_ARRAY;
		descriptor.resolution = { 256, 256, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
		m_shadowmapData.MapTargetCube = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D_ARRAY;
		descriptor.resolution = { ShadowmapData::TileSize, ShadowmapData::TileSize, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_NONE;
		m_shadowmapData.MapTargetProj = CreateRef<RenderTexture>(descriptor);

		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		m_shadowmapData.MapIntermediate = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D;
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_NONE;
		descriptor.resolution = { ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, 0 };
		m_shadowmapData.ShadowmapAtlas = CreateRef<RenderTexture>(descriptor);

		m_depthTiles = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::INT2, "DEPTHMINMAX"},
		}), ClusterCount, true, GL_NONE);
	
		m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4, "COLOR"},
			{CG_TYPE::FLOAT4, "DIRECTION"},
			{CG_TYPE::UINT, "SHADOWMAP_INDEX"},
			{CG_TYPE::UINT, "PROJECTION_INDEX"},
			{CG_TYPE::UINT, "COOKIE_INDEX"},
			{CG_TYPE::UINT, "TYPE"},
		}), 32, false, GL_STREAM_DRAW);
	
		m_lightMatricesBuffer = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4X4, "MATRIX"}
		}), 32, false, GL_STREAM_DRAW);

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

	void LightsManager::RenderShadowmapsForLightType(PK::ECS::EntityDatabase* entityDb, 
		Shader* renderShadows,
		Shader* blurshadowmap,
		RenderTexture* target,
		BufferView<uint>& lightIndices, 
		LightType type)
	{
		auto batchIdx = 0u;
		auto lightIndex = 0u;
		const auto cullingMask = (ushort)(ECS::Components::RenderHandleFlags::Renderer | ECS::Components::RenderHandleFlags::ShadowCaster);
		const auto maxBatches = (ShadowmapData::TileCountPerAxis * ShadowmapData::TileCountPerAxis) / ShadowmapData::BatchSize;
		ECS::EntityViews::LightRenderable* batchViews[ShadowmapData::BatchSize];

		while (batchIdx < maxBatches && lightIndex < lightIndices.count)
		{
			auto atlasBaseIdx = batchIdx * ShadowmapData::BatchSize;
			auto batchSize = 0u;
			auto zfar = 0.0f;
			++batchIdx;

			while (batchSize < ShadowmapData::BatchSize && lightIndex < lightIndices.count)
			{
				auto* lightview = entityDb->Query<ECS::EntityViews::LightRenderable>(ECS::EGID(lightIndices[lightIndex++], (uint)ECS::ENTITY_GROUPS::ACTIVE));

				if (lightview->light->lightType == type && lightview->light->castShadows)
				{
					batchViews[batchSize++] = lightview;
				}
			}

			if (batchSize == 0)
			{
				continue;
			}

			Batching::ResetCollection(&m_shadowmapData.Batches);

			for (uint i = 0; i < batchSize; ++i)
			{
				auto* lightview = batchViews[i];
				auto radius = lightview->light->radius;
				auto baseKey = ((uint)i << 16u) | (lightview->light->linearIndex & 0xFFFF);

				if (radius > zfar)
				{
					zfar = radius;
				}

				ShadowmapContext ctx = { &m_shadowmapData, baseKey };

				switch (type)
				{
					case LightType::Point:
					{
						auto bounds = entityDb->Query<ECS::EntityViews::BaseRenderable>(batchViews[i]->GID)->bounds->worldAABB;
						Culling::ExecuteOnVisibleItemsCubeFaces(entityDb, bounds, cullingMask, OnCullVisibleShadowmap, &ctx);
						break;
					}
					case LightType::Spot:
					{
						auto projection = Functions::GetPerspective(lightview->light->angle, 1.0f, 0.1f, lightview->light->radius) * lightview->transform->worldToLocal;
						Culling::ExecuteOnVisibleItemsFrustum(entityDb, projection, cullingMask, OnCullVisibleShadowmap, &ctx);
						break;
					}
				}
			}

			Batching::UpdateBuffers(&m_shadowmapData.Batches);

			GraphicsAPI::SetRenderTarget(target, false);
			GraphicsAPI::Clear(float4(zfar, zfar * zfar, 0, 0), 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Batching::DrawBatches(&m_shadowmapData.Batches, 0, renderShadows, m_properties);

			m_properties.SetKeywords({ StringHashID::StringToID("SHADOW_BLUR_PASS0") });
			GraphicsAPI::SetRenderTarget(m_shadowmapData.MapTargetProj.get(), false);
			GraphicsAPI::BlitInstanced(0, batchSize, blurshadowmap, m_properties);

			m_properties.SetKeywords({ StringHashID::StringToID("SHADOW_BLUR_PASS1") });
			GraphicsAPI::SetRenderTarget(m_shadowmapData.ShadowmapAtlas.get(), false);
			GraphicsAPI::BlitInstanced(atlasBaseIdx, batchSize, blurshadowmap, m_properties);
		}
	}

	void LightsManager::UpdateShadowmaps(ECS::EntityDatabase* entityDb, BufferView<uint>& lightIndices)
	{
		auto shaderRenderCube = m_shadowmapData.ShaderRenderMapCube;
		auto shaderRenderProj = m_shadowmapData.ShaderRenderMapProj;
		auto shaderBlurCube = m_shadowmapData.ShaderBlurCube;
		auto shaderBlurProj = m_shadowmapData.ShaderBlurProj;

		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatchCube"), m_shadowmapData.MapTargetCube->GetColorBuffer(0)->GetGraphicsID());
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatch0"), m_shadowmapData.MapTargetProj->GetColorBuffer(0)->GetGraphicsID());
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatch1"), m_shadowmapData.MapIntermediate->GetColorBuffer(0)->GetGraphicsID());

		float4 viewports[3] = 
		{
			{0, 0, 256, 256},
			{0, 0, ShadowmapData::TileSize, ShadowmapData::TileSize},
			{0, 0, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis},
		};

		GraphicsAPI::SetViewPorts(0, viewports, 3);

		RenderShadowmapsForLightType(entityDb, shaderRenderCube, shaderBlurCube, m_shadowmapData.MapTargetCube.get(), lightIndices, LightType::Point);
		RenderShadowmapsForLightType(entityDb, shaderRenderProj, shaderBlurProj, m_shadowmapData.MapIntermediate.get(), lightIndices, LightType::Spot);
	}

	void LightsManager::UpdateLightBuffers(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights)
	{
		m_lightsBuffer->ValidateSize((uint)visibleLights.count + 1);

		auto lightProjectionCount = 0;

		auto lightsBuffer = m_lightsBuffer->BeginMapBufferRange<Structs::PKRawLight>(0, visibleLights.count + 1);

		for (size_t i = 0; i < visibleLights.count; ++i)
		{
			auto* view = entityDb->Query<ECS::EntityViews::LightRenderable>(ECS::EGID(visibleLights[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));
			view->light->linearIndex = (uint)i;
			view->light->shadowmapIndex = view->light->castShadows ? (uint)i : 0xFFFFFFFF;
			view->light->projectionIndex = view->light->lightType != LightType::Point ? lightProjectionCount++ : 0xFFFFFFFF;

			lightsBuffer[i] = 
			{ 
				view->light->color, 
				float4(view->transform->position, view->light->radius), 
				view->light->shadowmapIndex, 
				view->light->projectionIndex, 
				(uint)view->light->cookie, 
				(uint)view->light->lightType 
			};
		}

		lightsBuffer[visibleLights.count] = { CG_COLOR_CLEAR, CG_FLOAT4_ZERO, 0u, 0u, 0u, 0u };
		m_lightsBuffer->EndMapBuffer();

		if (lightProjectionCount > 0)
		{
			m_lightMatricesBuffer->ValidateSize((uint)lightProjectionCount);

			auto projectionIndex = 0;
			auto matricesBuffer = m_lightMatricesBuffer->BeginMapBufferRange<float4x4>(0, lightProjectionCount);

			for (size_t i = 0; i < visibleLights.count; ++i)
			{
				auto* view = entityDb->Query<ECS::EntityViews::LightRenderable>(ECS::EGID(visibleLights[i], (uint)ECS::ENTITY_GROUPS::ACTIVE));

				switch (view->light->lightType)
				{
					case LightType::Spot:
					{
						matricesBuffer[projectionIndex++] = Functions::GetPerspective(view->light->angle, 1.0f, 0.1f, view->light->radius) * view->transform->worldToLocal;
					}
					break;
				}
			}

			m_lightMatricesBuffer->EndMapBuffer();
		}
	}
	
	void LightsManager::Preprocess(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution, float znear, float zfar)
	{
		UpdateLightBuffers(entityDb, visibleLights);

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
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_LightMatrices, m_lightMatricesBuffer->GetGraphicsID());
		GraphicsAPI::SetGlobalFloat(hashCache->pk_ClusterFrustumInfo, frustuminfo, 5);
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_GlobalLightsList, m_globalLightsList->GetGraphicsID());
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_LightTiles, m_lightTiles->GetGraphicsID());
		m_properties.SetComputeBuffer(hashCache->pk_FDepthRanges, m_depthTiles->GetGraphicsID());
		m_properties.SetComputeBuffer(hashCache->pk_ClusterDispatchInfo, m_clusterDispatchInfo->GetGraphicsID());
		m_properties.SetComputeBuffer(hashCache->pk_VisibleClusters, m_VisibleClusterList->GetGraphicsID());
	
		uint zero[5] = { 0, 0, 0, 0, 0 };
		m_clusterDispatchInfo->SubmitData(&zero, 0, CG_TYPE_SIZE_INT * 5);

		UpdateShadowmaps(entityDb, visibleLights);
	}
	
	void LightsManager::UpdateLightTiles(const uint2& resolution)
	{
		auto depthCountX = (uint)std::ceilf(resolution.x / 16.0f);
		auto depthCountY = (uint)std::ceilf(resolution.y / 16.0f);
	
		GraphicsAPI::DispatchCompute(m_computeDepthReset, { 1,1, GridSizeZ / 4 }, m_properties);
		GraphicsAPI::DispatchCompute(m_computeDepthTiles, { depthCountX, depthCountY, 1 }, m_properties);
		
		m_properties.SetKeywords({ m_passKeywords[0] });
		GraphicsAPI::DispatchCompute(m_computeCullClusters, { GridSizeX, GridSizeY, GridSizeZ }, m_properties);
	
		m_properties.SetKeywords({ m_passKeywords[1] });
		GraphicsAPI::DispatchCompute(m_computeCullClusters, { 1, 1, 1}, m_properties);
		
		GraphicsAPI::DispatchComputeIndirect(m_computeLightAssignment, m_clusterDispatchInfo->GetGraphicsID(), 0, m_properties);
	}
	
	void LightsManager::DrawDebug() { GraphicsAPI::Blit(m_debugVisualize, m_properties); }
}