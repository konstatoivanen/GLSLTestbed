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
            void OnPreRender(const RenderTexture* source) final;
            void Execute(const RenderTexture* source, const RenderTexture* destination) final;
        private:
            ECS::EntityDatabase* m_entityDb;
            Shader* m_shaderVoxelize;
            Shader* m_computeMipmap;
            Ref<RenderBuffer> m_voxelsDiffuse;
            Ref<RenderTexture> m_screenSpaceGI;
            Batching::MeshBatchCollection m_batches;
    };
}
