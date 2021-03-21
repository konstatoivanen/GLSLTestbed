#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterBloom : public FilterBase
    {
        public:
            FilterBloom(Weak<Shader> shader, Weak<TextureXD> lensDirt, float exposure, float intensity, float lensDirtIntensity);
            void Execute(Ref<RenderTexture> source, Ref<RenderTexture> destination) override;
    
        private:
            float m_exposure;
            float m_intensity;
            float m_lensDirtIntensity;
            Weak<TextureXD> m_lensDirtTexture;
            Ref<RenderTexture> m_blurTextures[6];
            uint m_passKeywords[4];
            GraphicsID m_bloomLayers[6];
    };
}