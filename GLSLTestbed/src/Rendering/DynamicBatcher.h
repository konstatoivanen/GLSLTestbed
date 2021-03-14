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
        Ref<ComputeBuffer> instancingBuffer;
        std::vector<float4x4> matrices;
        size_t count;
    };

    public:
        void Reset();
        void QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, const float4x4& matrix);
        void UpdateBuffers();
        void Execute();
        void Execute(Ref<Material>& overrideMaterial);
    private:
        std::vector<DynamicBatch> m_batches;
        std::map<ulong, uint> m_batchmap;
};