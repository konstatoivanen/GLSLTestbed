#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"
#include "Core/ApplicationConfig.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterVolumetricFog : public FilterBase
    {
        public:
            FilterVolumetricFog(AssetDatabase* assetDatabase, const ApplicationConfig* config);
            void Execute(const RenderTexture* source, const RenderTexture* destination);
            void OnUpdateParameters(const ApplicationConfig* config);

        private:
            Utilities::Ref<ConstantBuffer> m_volumeResources;
            Utilities::Ref<ComputeBuffer> m_depthTiles;
            Utilities::Ref<RenderBuffer> m_volumeInject;
            Utilities::Ref<RenderBuffer> m_volumeScatter;
            Shader* m_computeInject = nullptr;
            Shader* m_computeScatter = nullptr;
            Shader* m_computeDepthTiles = nullptr;
    };
}