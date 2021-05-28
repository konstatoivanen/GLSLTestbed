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
            FilterVolumetricFog(AssetDatabase* assetDatabase, const ApplicationConfig& config);
            void OnPreRender(const RenderTexture* source) override;
            void Execute(const RenderTexture* source, const RenderTexture* destination) override;

        private:
            Utilities::Ref<ConstantBuffer> m_volumeResources;
            Utilities::Ref<ComputeBuffer> m_depthTiles;
            Utilities::Ref<RenderBuffer> m_volumeLightDensity;
            Utilities::Ref<RenderBuffer> m_volumeScatter;
            Shader* m_computeInject = nullptr;
            Shader* m_computeScatter = nullptr;
            Shader* m_computeDepthTiles = nullptr;
    };
}