#pragma once
#include "Core/BufferView.h"
#include "ECS/Sequencer.h"
#include "Core/IService.h"
#include "Core/Time.h"
#include "ECS/EntityDatabase.h"
#include "Core/ApplicationConfig.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Structs/GraphicsContext.h"
#include "Rendering/Structs/StructsCommon.h"
#include "Rendering/DynamicBatcher.h"
#include "Rendering/FrustumCuller.h"
#include "Rendering/PostProcessing/FilterBloom.h"
#include "Rendering/PostProcessing/FilterAO.h"

namespace PK::Rendering
{
    class RenderPipeline : public IService, public PK::ECS::ISimpleStep, public PK::ECS::IStep<Time>
    {
        public:
            RenderPipeline(AssetDatabase* assetDatabase, PK::ECS::EntityDatabase* entityDb, const ApplicationConfig& config);
    
            void Step(Time* token) override;
            void Step(int condition) override;
    
        private:
            void OnPreRender();
            void OnRender();
            void OnPostRender();
    
            GraphicsContext m_context;  
            PK::ECS::EntityDatabase* m_entityDb;
            FrustumCuller m_frustrumCuller;
            DynamicBatcher m_dynamicBatcher;
            PostProcessing::FilterBloom m_filterBloom;
            PostProcessing::FilterAO m_filterAO;
    
            Utilities::Ref<RenderTexture> m_PreZRenderTarget;
            Utilities::Ref<RenderTexture> m_HDRRenderTarget;
            Utilities::Ref<ConstantBuffer> m_constantsPerFrame;
            Utilities::Ref<ComputeBuffer> m_lightsBuffer;
            Utilities::Weak<Shader> m_depthNormalsShader;
            Utilities::Weak<Shader> m_OEMBackgroundShader;
            Utilities::Weak<TextureXD> m_OEMTexture;
            float m_OEMExposure;
    };
}