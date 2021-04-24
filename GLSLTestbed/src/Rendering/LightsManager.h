#pragma once
#include "Utilities/Ref.h"
#include "Core/NoCopy.h"
#include "ECS/EntityDatabase.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"
#include "Rendering/Batching.h"
#include "Rendering/Culling.h"
#include "Rendering/Objects/Buffer.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Objects/TextureXD.h"
#include <hlslmath.h>

namespace PK::Rendering
{
    using namespace PK::Math;
    using namespace PK::Rendering::Objects;

    struct ShadowmapLightTypeData
    {
        Utilities::Ref<RenderTexture> SceneRenderTarget;
        Shader* ShaderRenderShadows;
        Shader* ShaderBlur;
        uint viewFirst;
        uint viewCount;
        uint atlasBaseIndex = 0;
    };
    
    struct ShadowmapData
    {
        ShadowmapLightTypeData LightIndices[(int)LightType::TypeCount];
        Batching::IndexedMeshBatchCollection Batches;
        Utilities::Ref<RenderTexture> MapIntermediate;
        Utilities::Ref<RenderTexture> ShadowmapAtlas;

        static const uint BatchSize = 4;
        static const uint TileSize = 512;
        static const uint TileCountPerAxis = 8; 
        static const uint TotalTileCount = 8 * 8; 
    };

    class LightsManager : public PK::Core::NoCopy
    {
        public:
            LightsManager(AssetDatabase* assetDatabase);

            void Preprocess(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution, float znear, float zfar);

            void UpdateLightTiles(const uint2& resolution);

            void DrawDebug();

            const Ref<RenderTexture>& GetShadowmapAtlas() const { return m_shadowmapData.ShadowmapAtlas; }

        private:
            void UpdateShadowmaps(PK::ECS::EntityDatabase* entityDb);
            void UpdateLightBuffers(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights);

            const uint MaxLightsPerTile = 64;
            const uint GridSizeX = 16;
            const uint GridSizeY = 9;
            const uint GridSizeZ = 24;
            const uint ClusterCount = GridSizeX * GridSizeY * GridSizeZ;

            std::vector<PK::ECS::EntityViews::LightRenderable*> m_visibleLights;
            uint m_visibleLightCount;

            ShaderPropertyBlock m_properties;
            ShadowmapData m_shadowmapData;

            Shader* m_computeLightAssignment;
            Shader* m_computeDepthTiles;
            Shader* m_debugVisualize;

            Utilities::Ref<ComputeBuffer> m_lightsBuffer;
            Utilities::Ref<ComputeBuffer> m_lightMatricesBuffer;
            Utilities::Ref<ComputeBuffer> m_lightDirectionsBuffer;
            Utilities::Ref<ComputeBuffer> m_globalLightsList;
            Utilities::Ref<ComputeBuffer> m_globalLightIndex;
            Utilities::Ref<ComputeBuffer> m_lightTiles;
            Utilities::Ref<ComputeBuffer> m_depthTiles;
    };
}