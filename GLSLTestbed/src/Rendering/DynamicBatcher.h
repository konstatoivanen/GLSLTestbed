#pragma once
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/Mesh.h"

// @TODO Consider using persistently mapped triple buffering instead
class DynamicBatcher
{
    struct DynamicBatch
    {
        Weak<Material> material;
        Weak<Mesh> mesh;
        uint submesh;
        uint offset;
        uint count;
        uint activeIndex;
    };

    public:
        void ResetCapacities();
        void BuildCapacity(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material);
        void BeginMapBuffers();
        void EndMapBuffers();
        void QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, const float4x4& matrix);
        void Execute();
        void Execute(Ref<Material>& overrideMaterial);
    private:
        Ref<ComputeBuffer> m_instancingBuffer;
        std::vector<DynamicBatch> m_batches;
        std::map<ulong, uint> m_batchmap;
        BufferView<float4x4> m_mappedBuffer;
};