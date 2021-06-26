#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"
#include "Core/ApplicationConfig.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterAO : public FilterBase
    {
        public:
            FilterAO(AssetDatabase* assetDatabase, const ApplicationConfig* config);
            void OnPreRender(const RenderTexture* source);
            void Execute();

        private:
            float m_intensity = 0.0f;
            float m_radius = 0.0f;
            bool m_downsample = false;
            bool m_updateParams = false;
            Ref<RenderTexture> m_renderTargets[2];
            Ref<ComputeBuffer> m_passBuffer;
            uint m_passKeywords[2];
    };
}