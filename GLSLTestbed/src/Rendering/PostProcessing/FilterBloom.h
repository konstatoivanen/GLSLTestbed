#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"
#include "Core/ApplicationConfig.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterBloom : public FilterBase
    {
        public:
            FilterBloom(AssetDatabase* assetDatabase, const ApplicationConfig& config);
            void OnPreRender(const RenderTexture* source) override;
            void Execute(const RenderTexture* source, const RenderTexture* destination) override;
    
        private:
            Shader* m_computeHistogram = nullptr;
            Shader* m_computeFilmgrain = nullptr;
            Ref<RenderTexture> m_renderTargets[6];
            Ref<RenderTexture> m_filmGrainTexture;
            Ref<ComputeBuffer> m_passBuffer;
            Ref<ComputeBuffer> m_histogram;
            Ref<ConstantBuffer> m_paramatersBuffer;
            bool m_updateParameters;
            uint m_passKeywords[5];
    };
}