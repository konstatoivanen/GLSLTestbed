#pragma once
#include "Rendering/PostProcessing/FilterBase.h"
#include "Core/NoCopy.h"
#include "ECS/EntityDatabase.h"
#include "Rendering/Batching.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Core/ApplicationConfig.h"
#include <hlslmath.h>

namespace PK::Rendering::PostProcessing
{
    using namespace PK::Math;
    using namespace PK::Rendering::Objects;
    
    class FilterSceneGI : public FilterBase
    {
        public: 
            FilterSceneGI(AssetDatabase* assetDatabase, ECS::EntityDatabase* entityDb, const ApplicationConfig* config);
            void OnPreRender(const RenderTexture* source);
            void Execute(Batching::DynamicBatchCollection* visibleBatches);
        private:
            ECS::EntityDatabase* m_entityDb;
            Shader* m_shaderVoxelize;
            Shader* m_computeMipmap;
            bool m_downscale;
            int m_rasterAxis;
            Ref<RenderBuffer> m_voxelsDiffuse;
            Ref<RenderTexture> m_screenSpaceGI;
    };
}
