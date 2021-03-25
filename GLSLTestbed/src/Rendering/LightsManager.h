#pragma once
#include "Utilities/Ref.h"
#include "Rendering/Objects/Buffer.h"
#include "Core/NoCopy.h"
#include "ECS/EntityDatabase.h"
#include <hlslmath.h>

namespace PK::Rendering
{
    using namespace PK::Math;
    using namespace PK::Rendering::Objects;

    class LightsManager : public PK::Core::NoCopy
    {
        public:
            LightsManager(AssetDatabase* assetDatabase);

            void Update(PK::ECS::EntityDatabase* entityDb, Core::BufferView<uint> visibleLights, const uint2& resolution, float znear, float zfar);

            void UpdateLightTiles(const uint2& resolution);

        private:
            const uint MaxLightsPerTile = 64;
            const uint GridSizeX = 16;
            const uint GridSizeY = 9;
            const uint GridSizeZ = 24;
            const uint ClusterCount = GridSizeX * GridSizeY * GridSizeZ;

            uint m_passKeywords[2];

            ShaderPropertyBlock m_properties;
            Utilities::Weak<Shader> m_computeLightAssignment;
            Utilities::Weak<Shader> m_computeCullClusters;
            Utilities::Weak<Shader> m_computeDepthTiles;
            Utilities::Weak<Shader> m_computeDepthReset;

            Utilities::Ref<ComputeBuffer> m_lightsBuffer;
            Utilities::Ref<ComputeBuffer> m_globalLightsList;
            Utilities::Ref<ComputeBuffer> m_lightTiles;

            Utilities::Ref<ComputeBuffer> m_depthTiles;
            Utilities::Ref<ComputeBuffer> m_VisibleClusterList;
            Utilities::Ref<ComputeBuffer> m_clusterDispatchInfo;
    };
}