#include "PrecompiledHeader.h"
#include "Utilities/Utilities.h"
#include "Utilities/HashCache.h"
#include "LightsManager.h"

namespace PK::Rendering
{
	struct ShadowmapContext
	{
		ShadowmapData* data;
		uint index;
	};

	static int LightViewCompare(PK::ECS::EntityViews::LightRenderable* a, PK::ECS::EntityViews::LightRenderable* b)
	{
		if (a->light->castShadows < b->light->castShadows)
		{
			return -1;
		}

		if (a->light->castShadows > b->light->castShadows)
		{
			return 1;
		}

		if (a->light->lightType < b->light->lightType)
		{
			return -1;
		}

		if (a->light->lightType > b->light->lightType)
		{
			return 1;
		}

		return 0;
	}

	static void QuickSortVisibleLights(PK::ECS::EntityViews::LightRenderable** arr, int low, int high)
	{
		int i = low;
		int j = high;
		auto pivot = arr[(i + j) / 2];

		while (i <= j)
		{
			while (LightViewCompare(arr[i], pivot) < 0)
			{
				i++;
			}

			while (LightViewCompare(arr[j], pivot) > 0)
			{
				j--;
			}

			if (i <= j)
			{
				auto temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
				i++;
				j--;
			}
		}

		if (j > low)
		{
			QuickSortVisibleLights(arr, low, j);
		}

		if (i < high)
		{
			QuickSortVisibleLights(arr, i, high);
		}
	}

	static void OnCullVisibleShadowmap(ECS::EntityDatabase* entityDb, ECS::EGID egid, uint clipIndex, float depth, void* context)
	{
		auto ctx = reinterpret_cast<ShadowmapContext*>(context);
		auto renderable = entityDb->Query<ECS::EntityViews::MeshRenderable>(egid);
		auto index = (clipIndex << 24u) | ctx->index;
		Batching::QueueDraw(&ctx->data->Batches, renderable->mesh->sharedMesh, { &renderable->transform->localToWorld, depth, index });
	}

	LightsManager::LightsManager(AssetDatabase* assetDatabase, const ApplicationConfig& config) : m_cascadeLinearity(config.CascadeLinearity)
	{
		m_computeLightAssignment = assetDatabase->Find<Shader>("CS_ClusteredLightAssignment");
		m_computeDepthTiles = assetDatabase->Find<Shader>("CS_ClusteredDepthMax");
		m_debugVisualize = assetDatabase->Find<Shader>("SH_VS_ClusterDebug");
	
		m_shadowmapTileSize = config.ShadowmapTileSize;
		m_shadowmapTileCount = config.ShadowmapTileCount;
		m_shadowmapCubeFaceSize = (uint)sqrt((m_shadowmapTileSize * m_shadowmapTileSize) / 6);

		m_shadowmapData.LightIndices[(int)LightType::Point].ShaderRenderShadows = assetDatabase->Find<Shader>("SH_WS_ShadowmapCube");
		m_shadowmapData.LightIndices[(int)LightType::Spot].ShaderRenderShadows = assetDatabase->Find<Shader>("SH_WS_ShadowmapPersp");
		m_shadowmapData.LightIndices[(int)LightType::Directional].ShaderRenderShadows = assetDatabase->Find<Shader>("SH_WS_ShadowmapOrtho");
		m_shadowmapData.LightIndices[(int)LightType::Point].ShaderBlur = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurCube");
		m_shadowmapData.LightIndices[(int)LightType::Spot].ShaderBlur = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurProj");
		m_shadowmapData.LightIndices[(int)LightType::Directional].ShaderBlur = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurProj");

		m_shadowmapData.LightIndices[(int)LightType::Point].maxBatchSize = ShadowmapData::BatchSize;
		m_shadowmapData.LightIndices[(int)LightType::Spot].maxBatchSize = ShadowmapData::BatchSize;
		m_shadowmapData.LightIndices[(int)LightType::Directional].maxBatchSize = 1;

		auto descriptor = RenderTextureDescriptor();
		descriptor.dimension = GL_TEXTURE_CUBE_MAP_ARRAY;
		descriptor.resolution = { m_shadowmapCubeFaceSize, m_shadowmapCubeFaceSize, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
		m_shadowmapData.LightIndices[(int)LightType::Point].SceneRenderTarget = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D_ARRAY;
		descriptor.resolution = { m_shadowmapTileSize, m_shadowmapTileSize, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		m_shadowmapData.LightIndices[(int)LightType::Directional].SceneRenderTarget = m_shadowmapData.LightIndices[(int)LightType::Spot].SceneRenderTarget = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D_ARRAY;
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_NONE;
		descriptor.resolution = { m_shadowmapTileSize, m_shadowmapTileSize, m_shadowmapTileCount + ShadowmapData::BatchSize };
		m_shadowmapData.ShadowmapAtlas = CreateRef<RenderTexture>(descriptor);

		TextureDescriptor imageDescriptor;
		imageDescriptor.dimension = GL_TEXTURE_3D;
		imageDescriptor.colorFormat = GL_R32UI;
		imageDescriptor.miplevels = 0;
		imageDescriptor.filtermag = GL_NEAREST;
		imageDescriptor.filtermin = GL_NEAREST;
		imageDescriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		imageDescriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		imageDescriptor.wrapmodez = GL_CLAMP_TO_EDGE;
		imageDescriptor.resolution = { GridSizeX, GridSizeY, GridSizeZ };
		m_lightTiles = CreateRef<RenderBuffer>(imageDescriptor);

		m_lightsBuffer = CreateRef<ComputeBuffer>(BufferLayout(
		{
			{CG_TYPE::FLOAT4, "COLOR"},
			{CG_TYPE::FLOAT4, "DIRECTION"},
			{CG_TYPE::UINT, "SHADOWMAP_INDEX"},
			{CG_TYPE::UINT, "PROJECTION_INDEX"},
			{CG_TYPE::UINT, "COOKIE_INDEX"},
			{CG_TYPE::UINT, "TYPE"},
		}), 32, false, GL_STREAM_DRAW);
	
		m_depthTiles = CreateRef<ComputeBuffer>(BufferLayout({{CG_TYPE::UINT, "DEPTHMAX"}}), GridSizeX * GridSizeY, true, GL_NONE);
		m_lightMatricesBuffer = CreateRef<ComputeBuffer>(BufferLayout({{CG_TYPE::FLOAT4X4, "MATRIX"}}), 32, false, GL_STREAM_DRAW);
		m_lightDirectionsBuffer = CreateRef<ComputeBuffer>(BufferLayout({{CG_TYPE::FLOAT4, "DIRECTION"}}), 32, false, GL_STREAM_DRAW);
		m_globalLightsList = CreateRef<ComputeBuffer>(BufferLayout({{CG_TYPE::INT, "INDEX"}}), ClusterCount * MaxLightsPerTile, true, GL_NONE);
		m_globalLightIndex = CreateRef<ComputeBuffer>(BufferLayout({{CG_TYPE::UINT, "INDEX"}}), 1, false, GL_STREAM_DRAW);
		m_properties.SetComputeBuffer(HashCache::Get()->pk_TileMaxDepths, m_depthTiles->GetGraphicsID());
		m_properties.SetComputeBuffer(HashCache::Get()->pk_LightDirections, m_lightDirectionsBuffer->GetGraphicsID());
		m_properties.SetComputeBuffer(HashCache::Get()->pk_GlobalListListIndex, m_globalLightIndex->GetGraphicsID());
	}

	void LightsManager::UpdateShadowmaps(ECS::EntityDatabase* entityDb, const float4x4& inverseViewProjection, float zNear, float zFar)
	{
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatchCube"), m_shadowmapData.LightIndices[(int)LightType::Point].SceneRenderTarget->GetColorBuffer(0)->GetGraphicsID());
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatch0"), m_shadowmapData.LightIndices[(int)LightType::Spot].SceneRenderTarget->GetColorBuffer(0)->GetGraphicsID());
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatch1"), m_shadowmapData.ShadowmapAtlas->GetColorBuffer(0)->GetGraphicsID());

		float4 viewports[2] = 
		{
			{0, 0, m_shadowmapCubeFaceSize, m_shadowmapCubeFaceSize},
			{0, 0, m_shadowmapTileSize, m_shadowmapTileSize},
		};

		const auto cullingMask = (ushort)(ECS::Components::RenderHandleFlags::Renderer | ECS::Components::RenderHandleFlags::ShadowCaster);

		GraphicsAPI::SetViewPorts(0, viewports, 2);

		for (auto typeIdx = 0; typeIdx < (int)LightType::TypeCount; ++typeIdx)
		{
			auto& typedata = m_shadowmapData.LightIndices[typeIdx];
			auto batchCount = (uint)std::ceil(typedata.viewCount / (float)typedata.maxBatchSize);

			for (auto batch = 0u; batch < batchCount; ++batch)
			{
				auto batchSize = std::min(typedata.viewCount - batch * typedata.maxBatchSize, typedata.maxBatchSize);
				auto tileCount = (batchSize * ShadowmapData::BatchSize) / typedata.maxBatchSize;
				auto baseLightIndex = typedata.viewFirst + batch * typedata.maxBatchSize;
				auto atlasIndex = m_visibleLights[baseLightIndex]->light->shadowmapIndex;
				auto maxDistance = 0.0f;

				Batching::ResetCollection(&m_shadowmapData.Batches);

				for (uint i = 0; i < batchSize; ++i)
				{
					auto* lightview = m_visibleLights[baseLightIndex + i];
					auto radius = lightview->light->radius;
					auto baseKey = ((uint)i << 16u) | (lightview->light->linearIndex & 0xFFFF);

					ShadowmapContext ctx = { &m_shadowmapData, baseKey };

					switch ((LightType)typeIdx)
					{
						case LightType::Point:
						{
							if (radius > maxDistance)
							{
								maxDistance = radius;
							}

							auto bounds = entityDb->Query<ECS::EntityViews::BaseRenderable>(lightview->GID)->bounds->worldAABB;
							Culling::ExecuteOnVisibleItemsCubeFaces(entityDb, bounds, cullingMask, OnCullVisibleShadowmap, &ctx);
							break;
						}
						case LightType::Spot:
						{
							if (radius > maxDistance)
							{
								maxDistance = radius;
							}

							auto projection = Functions::GetPerspective(lightview->light->angle, 1.0f, 0.1f, lightview->light->radius) * lightview->transform->worldToLocal;
							Culling::ExecuteOnVisibleItemsFrustum(entityDb, projection, cullingMask, OnCullVisibleShadowmap, &ctx);
							break;
						}
						case LightType::Directional:
						{
							float lightNear, lightFar;
							float4x4 cascades[ShadowmapData::BatchSize];
							Functions::GetShadowCascadeMatrices(
								lightview->transform->worldToLocal, 
								inverseViewProjection, 
								zNear, 
								zFar, 
								m_cascadeLinearity,
								-lightview->light->radius, 
								ShadowmapData::BatchSize, 
								cascades, 
								&lightNear, 
								&lightFar);

							Culling::ExecuteOnVisibleItemsCascades(entityDb, cascades, ShadowmapData::BatchSize, cullingMask, OnCullVisibleShadowmap, &ctx);
							
							// Directional Lights rely on the projection matrix zrange plane for max distance
							if (lightFar - lightNear > maxDistance)
							{
								maxDistance = lightFar - lightNear;
							}
							break;
						}
					}
				}

				Batching::UpdateBuffers(&m_shadowmapData.Batches);

				GraphicsAPI::SetRenderTarget(typedata.SceneRenderTarget.get(), false);
				GraphicsAPI::Clear(float4(maxDistance, maxDistance * maxDistance, 0, 0), 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				Batching::DrawBatches(&m_shadowmapData.Batches, 0, typedata.ShaderRenderShadows, m_properties);

				m_properties.SetKeywords({ StringHashID::StringToID("SHADOW_BLUR_PASS0") });
				GraphicsAPI::SetRenderTarget(m_shadowmapData.ShadowmapAtlas.get(), false);
				GraphicsAPI::BlitInstanced(atlasIndex + ShadowmapData::BatchSize, tileCount, typedata.ShaderBlur, m_properties);

				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
				m_properties.SetKeywords({ StringHashID::StringToID("SHADOW_BLUR_PASS1") });
				GraphicsAPI::BlitInstanced(atlasIndex, tileCount, typedata.ShaderBlur, m_properties);
			}
		}
	}

	void LightsManager::UpdateLightBuffers(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const float4x4& inverseViewProjection, float znear, float zfar)
	{
		m_visibleLightCount = 0;

		for (size_t i = 0; i < visibleLights.count; ++i)
		{
			Utilities::PushVectorElement(m_visibleLights, &m_visibleLightCount, entityDb->Query<ECS::EntityViews::LightRenderable>(ECS::EGID(visibleLights[i], (uint)ECS::ENTITY_GROUPS::ACTIVE)));
		}

		if (m_visibleLightCount > 1)
		{
			QuickSortVisibleLights(m_visibleLights.data(), 0, m_visibleLightCount - 1);
		}

		for (auto i = 0; i < (int)LightType::TypeCount; ++i)
		{
			m_shadowmapData.LightIndices[i].viewCount = 0;
			m_shadowmapData.LightIndices[i].viewFirst = 0xFFFFFFFF;
		}

		auto lightProjectionCount = 0;
		auto shadowMapCount = 0u;

		// Get visible shadowmap tiles per light type so that tile indexing can be ordered by light type
		for (size_t i = 0; i < m_visibleLightCount; ++i)
		{
			auto* view = m_visibleLights.at(i);
			view->light->linearIndex = (uint)i;

			switch (view->light->lightType)
			{
				case LightType::Directional:
					view->light->projectionIndex = lightProjectionCount;
					lightProjectionCount += ShadowmapData::BatchSize;
					break;
				case LightType::Point: 
					view->light->projectionIndex = 0; 
					break;
				case LightType::Spot: 
					view->light->projectionIndex = lightProjectionCount++;
					break;
			}

			view->light->shadowmapIndex = 0xFFFFFFFF;

			if (!view->light->castShadows || shadowMapCount >= m_shadowmapTileCount)
			{
				continue;
			}

			switch (view->light->lightType)
			{
				case LightType::Point:
				case LightType::Spot:
					view->light->shadowmapIndex = shadowMapCount++;
					break;
				case LightType::Directional:
					if (m_shadowmapTileCount - shadowMapCount >= ShadowmapData::BatchSize)
					{
						view->light->shadowmapIndex = shadowMapCount;
						shadowMapCount += ShadowmapData::BatchSize;
						break;
					}
					
					// Not enough atlas space for 4 cascades
					continue;
			}
			
			auto& indicesView = m_shadowmapData.LightIndices[(uint)view->light->lightType];
			indicesView.viewFirst = std::min(indicesView.viewFirst, (uint)i);
			++indicesView.viewCount;
		}

		m_lightMatricesBuffer->ValidateSize((uint)lightProjectionCount);
		m_lightDirectionsBuffer->ValidateSize((uint)lightProjectionCount);
		m_lightsBuffer->ValidateSize((uint)m_visibleLightCount + 1);

		auto bufferLights = m_lightsBuffer->BeginMapBufferRange<Structs::PKRawLight>(0, m_visibleLightCount + 1);
		auto bufferMatrices = lightProjectionCount > 0 ? m_lightMatricesBuffer->BeginMapBufferRange<float4x4>(0, lightProjectionCount) : BufferView<float4x4>();
		auto bufferDirections = lightProjectionCount > 0 ? m_lightDirectionsBuffer->BeginMapBufferRange<float4>(0, lightProjectionCount) : BufferView<float4>();

		for (size_t i = 0; i < m_visibleLightCount; ++i)
		{
			auto* view = m_visibleLights.at(i);
			auto position = CG_FLOAT4_ZERO;

			switch (view->light->lightType)
			{
				case LightType::Directional:
					float lightNear, lightFar;
					Functions::GetShadowCascadeMatrices(
						view->transform->worldToLocal,
						inverseViewProjection,
						znear,
						zfar,
						m_cascadeLinearity,
						-view->light->radius,
						ShadowmapData::BatchSize,
						bufferMatrices.data + view->light->projectionIndex,
						&lightNear,
						&lightFar);

					position = float4(view->transform->rotation * CG_FLOAT3_FORWARD, lightFar - lightNear);
					break;

				case LightType::Point:
					position = float4(view->transform->position, view->light->radius);
					break;

				case LightType::Spot:
					position = float4(view->transform->position, view->light->radius);
					bufferMatrices[view->light->projectionIndex] = Functions::GetPerspective(view->light->angle, 1.0f, 0.1f, view->light->radius) * view->transform->worldToLocal;
					bufferDirections[view->light->projectionIndex] = float4(view->transform->rotation * CG_FLOAT3_FORWARD, view->light->angle * CG_FLOAT_DEG2RAD);
					break;
			}

			bufferLights[i] = 
			{ 
				view->light->color,
				position,
				view->light->shadowmapIndex, 
				view->light->projectionIndex, 
				(uint)view->light->cookie, 
				(uint)view->light->lightType 
			};
		}

		bufferLights[m_visibleLightCount] = { CG_COLOR_CLEAR, CG_FLOAT4_ZERO, 0xFFFFFFFF, 0u, 0xFFFFFFFF, 0xFFFFFFFF };
		m_lightsBuffer->EndMapBuffer();

		if (lightProjectionCount > 0)
		{
			m_lightMatricesBuffer->EndMapBuffer();
			m_lightDirectionsBuffer->EndMapBuffer();
		}
	}
	
	void LightsManager::Preprocess(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution, const float4x4& inverseViewProjection, float zNear, float zFar)
	{
		UpdateLightBuffers(entityDb, visibleLights, inverseViewProjection, zNear, zFar);

		auto hashCache = HashCache::Get();
		m_globalLightIndex->Clear();
		m_depthTiles->Clear();
		GraphicsAPI::SetGlobalInt(hashCache->pk_LightCount, m_visibleLightCount);
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_Lights, m_lightsBuffer->GetGraphicsID());
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_LightMatrices, m_lightMatricesBuffer->GetGraphicsID());
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_GlobalLightsList, m_globalLightsList->GetGraphicsID());
		GraphicsAPI::SetGlobalImage(hashCache->pk_LightTiles, m_lightTiles->GetImageBindDescriptor(GL_READ_WRITE, 0, 0, true));
		UpdateShadowmaps(entityDb, inverseViewProjection, zNear, zFar);
	}
	
	void LightsManager::UpdateLightTiles(const uint2& resolution)
	{	
		auto depthCountX = (uint)std::ceilf(resolution.x / DepthGroupSize);
		auto depthCountY = (uint)std::ceilf(resolution.y / DepthGroupSize);
		GraphicsAPI::DispatchCompute(m_computeDepthTiles, { depthCountX, depthCountY, 1 }, m_properties, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		GraphicsAPI::DispatchCompute(m_computeLightAssignment, { 1,1, GridSizeZ / 4 }, m_properties, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	}
}