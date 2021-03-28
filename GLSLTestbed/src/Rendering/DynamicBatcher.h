#pragma once
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Mesh.h"

namespace PK::Rendering
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    // @TODO Consider using persistently mapped triple buffering instead
    class DynamicBatcher
    {
        struct MaterialBatch
        {
            Weak<Material> material;
            std::vector<float4x4*> matrices;
            uint instancingOffset = 0;
            uint submesh = 0;
            uint count = 0;
        };

        struct MeshBatch
        {
            Weak<Mesh> mesh;
            std::vector<int> materialBatches;
            uint materialBatchCount = 0;
            uint instancingOffset = 0;
            uint count = 0;
        };
    
        public:
            void Reset();
            void QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, float4x4* localToWorld, float4x4* worldToLocal);
            void UpdateBuffers();
            void Execute(uint32_t renderQueueIndex);
            void Execute(uint32_t renderQueueIndex, Ref<Material>& overrideMaterial);
            void Execute(uint32_t renderQueueIndex, Ref<Shader>& overrideShader);
        private:
            std::vector<MeshBatch> m_meshBatches;
            std::vector<MaterialBatch> m_materialBatches;
            std::unordered_map<GraphicsID, uint> m_meshBatchMap;
            std::unordered_map<ulong, uint> m_materialBatchMap;
            Ref<ComputeBuffer> m_matrixBuffer;
            uint m_batchedMatrixCount;
    };
}