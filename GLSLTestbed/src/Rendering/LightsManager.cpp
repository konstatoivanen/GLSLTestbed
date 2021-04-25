#include "PrecompiledHeader.h"
#include "Utilities/Utilities.h"
#include "Utilities/HashCache.h"
#include "Rendering/GraphicsAPI.h"
#include "LightsManager.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"

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

	LightsManager::LightsManager(AssetDatabase* assetDatabase)
	{
		m_computeLightAssignment = assetDatabase->Find<Shader>("CS_ClusteredLightAssignment");
		m_computeDepthTiles = assetDatabase->Find<Shader>("CS_ClusteredDepthMax");
		m_debugVisualize = assetDatabase->Find<Shader>("SH_VS_ClusterDebug");
	
		m_shadowmapData.LightIndices[(int)LightType::Point].ShaderRenderShadows = assetDatabase->Find<Shader>("SH_WS_ShadowmapCube");
		m_shadowmapData.LightIndices[(int)LightType::Spot].ShaderRenderShadows = assetDatabase->Find<Shader>("SH_WS_ShadowmapProj");
		m_shadowmapData.LightIndices[(int)LightType::Point].ShaderBlur = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurCube");
		m_shadowmapData.LightIndices[(int)LightType::Spot].ShaderBlur = assetDatabase->Find<Shader>("SH_VS_ShadowmapBlurProj");

		auto descriptor = RenderTextureDescriptor();
		descriptor.dimension = GL_TEXTURE_CUBE_MAP_ARRAY;
		descriptor.resolution = { 256, 256, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		descriptor.wrapmodex = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodey = GL_CLAMP_TO_EDGE;
		descriptor.wrapmodez = GL_CLAMP_TO_EDGE;
		m_shadowmapData.LightIndices[(int)LightType::Point].SceneRenderTarget = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D_ARRAY;
		descriptor.resolution = { ShadowmapData::TileSize, ShadowmapData::TileSize, ShadowmapData::BatchSize };
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_DEPTH_COMPONENT16;
		m_shadowmapData.LightIndices[(int)LightType::Spot].SceneRenderTarget = CreateRef<RenderTexture>(descriptor);

		descriptor.depthFormat = GL_NONE;
		m_shadowmapData.MapIntermediate = CreateRef<RenderTexture>(descriptor);

		descriptor.dimension = GL_TEXTURE_2D;
		descriptor.colorFormats = { GL_RG32F };
		descriptor.depthFormat = GL_NONE;
		descriptor.resolution = { ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, 0 };
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

	void LightsManager::UpdateShadowmaps(ECS::EntityDatabase* entityDb)
	{
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatchCube"), m_shadowmapData.LightIndices[(int)LightType::Point].SceneRenderTarget->GetColorBuffer(0)->GetGraphicsID());
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatch0"), m_shadowmapData.LightIndices[(int)LightType::Spot].SceneRenderTarget->GetColorBuffer(0)->GetGraphicsID());
		m_properties.SetTexture(StringHashID::StringToID("_ShadowmapBatch1"), m_shadowmapData.MapIntermediate->GetColorBuffer(0)->GetGraphicsID());

		float4 viewports[3] = 
		{
			{0, 0, 256, 256},
			{0, 0, ShadowmapData::TileSize, ShadowmapData::TileSize},
			{0, 0, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis, ShadowmapData::TileSize * ShadowmapData::TileCountPerAxis},
		};

		const auto cullingMask = (ushort)(ECS::Components::RenderHandleFlags::Renderer | ECS::Components::RenderHandleFlags::ShadowCaster);

		GraphicsAPI::SetViewPorts(0, viewports, 3);

		for (auto typeIdx = 0; typeIdx < (int)LightType::TypeCount; ++typeIdx)
		{
			auto& typedata = m_shadowmapData.LightIndices[typeIdx];
			auto batchCount = (uint)std::ceil(typedata.viewCount / (float)ShadowmapData::BatchSize);

			for (auto batch = 0u; batch < batchCount; ++batch)
			{
				auto batchSize = std::min(typedata.viewCount - batch * ShadowmapData::BatchSize, ShadowmapData::BatchSize);
				auto baseLightIndex = typedata.viewFirst + batch * ShadowmapData::BatchSize;
				auto atlasIndex = m_visibleLights[baseLightIndex]->light->shadowmapIndex;
				auto zfar = 0.0f;


				Batching::ResetCollection(&m_shadowmapData.Batches);

				for (uint i = 0; i < batchSize; ++i)
				{
					auto* lightview = m_visibleLights[baseLightIndex + i];
					auto radius = lightview->light->radius;
					auto baseKey = ((uint)i << 16u) | (lightview->light->linearIndex & 0xFFFF);

					if (radius > zfar)
					{
						zfar = radius;
					}

					ShadowmapContext ctx = { &m_shadowmapData, baseKey };

					switch ((LightType)typeIdx)
					{
						case LightType::Point:
						{
							auto bounds = entityDb->Query<ECS::EntityViews::BaseRenderable>(lightview->GID)->bounds->worldAABB;
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

				GraphicsAPI::SetRenderTarget(typedata.SceneRenderTarget.get(), false);
				GraphicsAPI::Clear(float4(zfar, zfar * zfar, 0, 0), 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				Batching::DrawBatches(&m_shadowmapData.Batches, 0, typedata.ShaderRenderShadows, m_properties);

				m_properties.SetKeywords({ StringHashID::StringToID("SHADOW_BLUR_PASS0") });
				GraphicsAPI::SetRenderTarget(m_shadowmapData.MapIntermediate.get(), false);
				GraphicsAPI::BlitInstanced(0, batchSize, typedata.ShaderBlur, m_properties);

				m_properties.SetKeywords({ StringHashID::StringToID("SHADOW_BLUR_PASS1") });
				GraphicsAPI::SetRenderTarget(m_shadowmapData.ShadowmapAtlas.get(), false);
				GraphicsAPI::BlitInstanced(atlasIndex, batchSize, typedata.ShaderBlur, m_properties);
			}
		}
	}

	void LightsManager::UpdateLightBuffers(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights)
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

		// Get visible shadowmap tiles per light type so that tile indexing can be ordered by light type
		for (size_t i = 0; i < m_visibleLightCount; ++i)
		{
			auto* view = m_visibleLights.at(i);
			view->light->linearIndex = (uint)i;
			view->light->projectionIndex = view->light->lightType != LightType::Point ? lightProjectionCount++ : 0;

			if (view->light->castShadows && i < ShadowmapData::TotalTileCount)
			{
				view->light->shadowmapIndex = (uint)i;
				auto& indicesView = m_shadowmapData.LightIndices[(uint)view->light->lightType];
				indicesView.viewFirst = std::min(indicesView.viewFirst, (uint)i);
				++indicesView.viewCount;
			}
			else
			{
				view->light->shadowmapIndex = 0xFFFFFFFF;
			}
		}

		m_lightMatricesBuffer->ValidateSize((uint)lightProjectionCount);
		m_lightDirectionsBuffer->ValidateSize((uint)lightProjectionCount);
		m_lightsBuffer->ValidateSize((uint)m_visibleLightCount + 1);

		auto lightsBuffer = m_lightsBuffer->BeginMapBufferRange<Structs::PKRawLight>(0, m_visibleLightCount + 1);
		auto matricesBuffer = lightProjectionCount > 0 ? m_lightMatricesBuffer->BeginMapBufferRange<float4x4>(0, lightProjectionCount) : BufferView<float4x4>();
		auto directionsBuffer = lightProjectionCount > 0 ? m_lightDirectionsBuffer->BeginMapBufferRange<float4>(0, lightProjectionCount) : BufferView<float4>();

		for (size_t i = 0; i < m_visibleLightCount; ++i)
		{
			auto* view = m_visibleLights.at(i);

			switch (view->light->lightType)
			{
				case LightType::Spot: 
					matricesBuffer[view->light->projectionIndex] = Functions::GetPerspective(view->light->angle, 1.0f, 0.1f, view->light->radius) * view->transform->worldToLocal; 
					directionsBuffer[view->light->projectionIndex] = float4(view->transform->rotation * CG_FLOAT3_FORWARD, view->light->angle * CG_FLOAT_DEG2RAD);
					break;
			}

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

		lightsBuffer[m_visibleLightCount] = { CG_COLOR_CLEAR, CG_FLOAT4_ZERO, 0u, 0u, 0u, 0u };
		m_lightsBuffer->EndMapBuffer();

		if (lightProjectionCount > 0)
		{
			m_lightMatricesBuffer->EndMapBuffer();
			m_lightDirectionsBuffer->EndMapBuffer();
		}
	}
	
	void LightsManager::Preprocess(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution)
	{
		UpdateLightBuffers(entityDb, visibleLights);

		auto hashCache = HashCache::Get();
	
		m_globalLightIndex->Clear();
		m_depthTiles->Clear();
		GraphicsAPI::SetGlobalInt(hashCache->pk_LightCount, m_visibleLightCount);
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_Lights, m_lightsBuffer->GetGraphicsID());
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_LightMatrices, m_lightMatricesBuffer->GetGraphicsID());
		GraphicsAPI::SetGlobalFloat(hashCache->pk_ClusterSizePx, std::ceilf(resolution.x / (float)GridSizeX));
		GraphicsAPI::SetGlobalComputeBuffer(hashCache->pk_GlobalLightsList, m_globalLightsList->GetGraphicsID());
		GraphicsAPI::SetGlobalImage(hashCache->pk_LightTiles, m_lightTiles->GetImageBindDescriptor(GL_READ_WRITE, 0, 0, true));
		UpdateShadowmaps(entityDb);
	}
	
	void LightsManager::UpdateLightTiles(const uint2& resolution)
	{
		auto depthCountX = (uint)std::ceilf(resolution.x / 16.0f);
		auto depthCountY = (uint)std::ceilf(resolution.y / 16.0f);
		GraphicsAPI::DispatchCompute(m_computeDepthTiles, { depthCountX, depthCountY, 1 }, m_properties, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		GraphicsAPI::DispatchCompute(m_computeLightAssignment, { 1,1, GridSizeZ / 4 }, m_properties, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	}
	
	void LightsManager::DrawDebug() 
	{ 
		GraphicsAPI::Blit(m_debugVisualize, m_properties);
	}
}