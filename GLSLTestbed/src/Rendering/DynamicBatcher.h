#pragma once
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/Mesh.h"

namespace PK::Rendering
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    // @TODO Consider using persistently mapped triple buffering instead
    class DynamicBatcher
    {
        struct DrawMatrices
        {
            float4x4* localToWorld;
            float4x4* worldToLocal;
        };

        struct DynamicBatch
        {
            Weak<Material> material;
            Weak<Mesh> mesh;
            uint submesh;
            Ref<ComputeBuffer> instancingBuffer;
            std::vector<DrawMatrices> matrices;
            size_t count;
        };
    
        public:
            void Reset();
            void QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, float4x4* localToWorld, float4x4* worldToLocal);
            void UpdateBuffers();
            void Execute(uint32_t renderQueueIndex);
            void Execute(uint32_t renderQueueIndex, Ref<Material>& overrideMaterial);
            void Execute(uint32_t renderQueueIndex, Ref<Shader>& overrideShader);
        private:
            std::vector<DynamicBatch> m_batches;
            std::map<ulong, uint> m_batchmap;
    };
}