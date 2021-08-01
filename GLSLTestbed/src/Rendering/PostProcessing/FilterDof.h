#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"
#include "Core/ApplicationConfig.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterDof : public FilterBase
    {
        public:
            FilterDof(AssetDatabase* assetDatabase, const ApplicationConfig* config);
            void OnPreRender(const RenderTexture* source);
            void Execute(const RenderTexture* source, const RenderTexture* destination);

        private:
            Ref<RenderTexture> m_renderTarget0;
            Ref<RenderTexture> m_renderTarget1;
            Ref<ConstantBuffer> m_paramsBuffer;
            Ref<ComputeBuffer> m_autoFocusBuffer;
            Shader* m_shaderComposite;
            Shader* m_shaderAutoFocus;
            uint m_passKeywords[2];
    };
}