#pragma once
#include "Core/BufferView.h"
#include "Core/Sequencer.h"
#include "Core/IService.h"
#include "Core/Time.h"
#include "Core/EntityDatabase.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Structs/GraphicsContext.h"
#include "Rendering/Structs/StructsCommon.h"
#include "Rendering/DynamicBatcher.h"

class FrustrumCuller
{
    struct CullingResults
    {
        std::vector<uint> list;
        size_t count;
    };

    public:
        void Update(PKECS::EntityDatabase* entityDb, const float4x4& matrix);
        BufferView<uint> GetCullingResults(uint type)
        { 
            auto& element = m_visibilityLists[type];
            return { element.list.data(), element.count };
        }

    private:
        std::map<uint, CullingResults> m_visibilityLists;
};

class RenderPipeline : public IService, public PKECS::ISimpleStep, public PKECS::IStep<Time>
{
    public:
        RenderPipeline(AssetDatabase* assetDatabase, PKECS::EntityDatabase* entityDb);

        void Step(Time* token) override;
        void Step(int condition) override;

    private:
        void OnPreRender();
        void OnRender();
        void OnPostRender();

        GraphicsContext m_context;  
        PKECS::EntityDatabase* m_entityDb;
        FrustrumCuller m_frustrumCuller;
        DynamicBatcher m_dynamicBatcher;

        Ref<RenderTexture> m_HDRRenderTarget;
        Ref<ConstantBuffer> m_constantsPerFrame;
        Ref<ComputeBuffer> m_lightsBuffer;
        Weak<Shader> m_OEMBackgroundShader;
        Weak<TextureXD> m_OEMTexture;
};