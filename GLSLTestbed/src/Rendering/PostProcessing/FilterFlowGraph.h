#pragma once
#include "Core/Input.h"
#include "Rendering/PostProcessing/FilterBase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Objects/Buffer.h"
#include "Core/ApplicationConfig.h"

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;

    class FilterFlowGraph : public FilterBase
    {
        public:
            FilterFlowGraph(AssetDatabase* assetDatabase, const ApplicationConfig* config);
            void Execute(const RenderTexture* destination);
            void OnInput(Input* input);

        private:
            Ref<RenderTexture> m_renderTarget;
            Ref<ComputeBuffer> m_boidsBuffer;

            Shader* m_shaderFlowIterate;
            Shader* m_shaderBoidUpdate;
            Shader* m_shaderDrawBoids;
    };
}