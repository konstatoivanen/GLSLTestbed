#pragma once
#include "Core/Sequencer.h"
#include "Core/IService.h"
#include "Core/Time.h"
#include "Rendering/Structs/GraphicsContext.h"
#include "Rendering/Structs/StructsCommon.h"

class RenderPipeline : public IService, public PKECS::ISimpleStep, public PKECS::IStep<Time>
{
    public:
        RenderPipeline(AssetDatabase* assetDatabase);

        void Step(Time* token) override;
        void Step(int condition) override;

    private:
        void OnPreRender();
        void OnRender();
        void OnPostRender();

        GraphicsContext m_context;  
        Ref<ConstantBuffer> m_constantsPerFrame;
        Ref<ComputeBuffer> m_lightsBuffer;
        std::vector<PKStructs::PKLight> m_lights;
        uint m_lightCount;
};