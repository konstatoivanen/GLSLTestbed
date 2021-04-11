#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterBloom : public FilterBase
    {
        public:
            FilterBloom(Weak<Shader> shader, Weak<TextureXD> lensDirt, float exposure, float intensity, float lensDirtIntensity);
            void OnPreRender(const RenderTexture* source) override;
            void Execute(const RenderTexture* source, const RenderTexture* destination) override;
    
        private:
            float m_exposure;
            float m_intensity;
            float m_lensDirtIntensity;
            Weak<TextureXD> m_lensDirtTexture;
            Ref<RenderTexture> m_renderTargets[6];
            Ref<ComputeBuffer> m_passBuffer;
            uint m_passKeywords[3];
    };
}