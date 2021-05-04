#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterAO : public FilterBase
    {
        public:
            FilterAO(Shader* shader, float intensity, float radius, bool downsample);
            void OnPreRender(const RenderTexture* source) override;
            void Execute(const RenderTexture* source, const RenderTexture* destination) override;

        private:
            float m_intensity = 0.0f;
            float m_radius = 0.0f;
            bool m_downsample = false;
            Ref<RenderTexture> m_renderTargets[2];
            Ref<ComputeBuffer> m_passBuffer;
            uint m_passKeywords[2];
    };
}