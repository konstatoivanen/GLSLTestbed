#pragma once
#include "Core/Sequencer.h"
#include "Core/IService.h"
#include "Core/Time.h"
#include "Rendering/Structs/GraphicsContext.h"

class RenderPipeline : public IService, public PKECS::ISimpleStep, public PKECS::IStep<Time>
{
    public:
        RenderPipeline(AssetDatabase* assetDatabase);

        void Step(Time* token) override;

        void Step(int condition) override;

        GraphicsContext m_context;       
};