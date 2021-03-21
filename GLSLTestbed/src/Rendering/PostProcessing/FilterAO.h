#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterAO : public FilterBase
    {
        public:
            FilterAO(Weak<Shader> shader, float intensity, float radius, bool downsample);
            void Execute(Ref<RenderTexture> source, Ref<RenderTexture> destination) override;

        private:
            float m_intensity;
            float m_radius;
            bool m_downsample;
            Ref<RenderTexture> m_renderTargets[2];
            uint m_passKeywords[3];
    };
}