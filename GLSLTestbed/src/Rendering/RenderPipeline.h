#pragma once
#include "Core/BufferView.h"
#include "ECS/Sequencer.h"
#include "Core/IService.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "ECS/EntityDatabase.h"
#include "Core/ApplicationConfig.h"
#include "Rendering/Objects/TextureXD.h"
#include "Rendering/Structs/GraphicsContext.h"
#include "Rendering/Structs/StructsCommon.h"
#include "Rendering/Batching.h"
#include "Rendering/Culling.h"
#include "Rendering/PostProcessing/FilterBloom.h"
#include "Rendering/PostProcessing/FilterAO.h"
#include "Rendering/PostProcessing/FilterVolumetricFog.h"
#include "Rendering/PostProcessing/FilterDof.h"
#include "Rendering/PostProcessing/FilterSceneGI.h"
#include "Rendering/LightsManager.h"

namespace PK::Rendering
{
    class RenderPipeline : public IService, 
                           public PK::ECS::ISimpleStep, 
                           public PK::ECS::IStep<Time>, 
                           public PK::ECS::IStep<Input>, 
                           public PK::ECS::IStep<AssetImportToken<ApplicationConfig>>
    {
        public:
            RenderPipeline(AssetDatabase* assetDatabase, PK::ECS::EntityDatabase* entityDb, const ApplicationConfig* config);
    
            void Step(Time* token) override;
            void Step(Input* token) override;
            void Step(int condition) override;
            void Step(AssetImportToken<ApplicationConfig>* token) override;
    
        private:
            void OnPreRender();
            void OnRender();
    
            bool m_enableLightingDebug;
            bool m_logframerate;

            GraphicsContext m_context;  
            PK::ECS::EntityDatabase* m_entityDb;
            Culling::VisibilityCache m_visibilityCache;
            Batching::DynamicBatchCollection m_dynamicBatches;
            LightsManager m_lightsManager;
            PostProcessing::FilterBloom m_filterBloom;
            PostProcessing::FilterAO m_filterAO;
            PostProcessing::FilterVolumetricFog m_filterFog;
            PostProcessing::FilterDof m_filterDof;
            PostProcessing::FilterSceneGI m_filterSceneGi;
    
            Utilities::Ref<RenderTexture> m_GeometryBufferTarget;
            Utilities::Ref<RenderTexture> m_HDRRenderTarget;
            Utilities::Ref<ConstantBuffer> m_constantsPerFrame;
            Shader* m_depthNormalsShader;
            Shader* m_OEMBackgroundShader;
            TextureXD* m_OEMTexture;
            float m_OEMExposure;
    };
}