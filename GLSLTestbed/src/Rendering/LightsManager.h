#pragma once
#include "Utilities/Ref.h"
#include "Core/NoCopy.h"
#include "ECS/EntityDatabase.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"
#include "Rendering/Batching.h"
#include "Rendering/Culling.h"
#include "Rendering/Objects/Buffer.h"
#include "Rendering/Objects/RenderTexture.h"
#include <hlslmath.h>

namespace PK::Rendering
{
    using namespace PK::Math;
    using namespace PK::Rendering::Objects;

    struct ShadowmapData
    {
        Batching::IndexedMeshBatchCollection Batches;
        Utilities::Ref<RenderTexture> ShadowmapCube;
        Utilities::Ref<RenderTexture> ShadowmapOctahedron;
        Utilities::Ref<RenderTexture> ShadowmapAtlas;

        Utilities::Weak<Shader> ShaderRenderCube;
        Utilities::Weak<Shader> ShaderBlurCube;

        static const uint BatchSize = 4;
        static const uint TileSize = 512;
        static const uint TileCountPerAxis = 8; 
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
            const uint MaxLightsPerTile = 64;
            const uint GridSizeX = 16;
            const uint GridSizeY = 9;
            const uint GridSizeZ = 24;
            const uint ClusterCount = GridSizeX * GridSizeY * GridSizeZ;

            uint m_passKeywords[2];

            ShaderPropertyBlock m_properties;
            ShadowmapData m_shadowmapData;

            Utilities::Weak<Shader> m_computeLightAssignment;
            Utilities::Weak<Shader> m_computeCullClusters;
            Utilities::Weak<Shader> m_computeDepthTiles;
            Utilities::Weak<Shader> m_computeDepthReset;
            Utilities::Weak<Shader> m_debugVisualize;

            Utilities::Ref<ComputeBuffer> m_lightsBuffer;
            Utilities::Ref<ComputeBuffer> m_globalLightsList;
            Utilities::Ref<ComputeBuffer> m_lightTiles;

            Utilities::Ref<ComputeBuffer> m_depthTiles;
            Utilities::Ref<ComputeBuffer> m_VisibleClusterList;
            Utilities::Ref<ComputeBuffer> m_clusterDispatchInfo;
    };
}