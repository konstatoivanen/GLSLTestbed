#pragma once
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Mesh.h"

namespace PK::Rendering::Batching
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;
        
    struct BatchBase
    {
        uint instancingOffset;
        uint drawCallCount;
    };

    struct MaterialBatch : BatchBase
    {
        Material* material;
        std::vector<float4x4*> matrices;
    };

    struct ShaderBatch : BatchBase
    {
        Ref<ComputeBuffer> instancedData;
        std::vector<uint> materialBatches;
        uint materialBatchCount = 0;
        int submesh = 0;
    };

    struct MeshBatch : BatchBase
    {
        Mesh* mesh;
        std::vector<uint> shaderBatches;
        uint shaderBatchCount = 0;
    };

    struct MeshOnlyBatch : BatchBase
    {
        Mesh* mesh;
        std::vector<float4x4*> matrices;
    };

    struct DynamicBatchCollection
    {
        std::vector<MaterialBatch> MaterialBatches;
        std::vector<ShaderBatch> ShaderBatches;
        std::vector<MeshBatch> MeshBatches;
        std::unordered_map<ulong, uint> BatchMap;

        // @TODO Consider using persistently mapped triple buffering instead
        Ref<ComputeBuffer> MatrixBuffer;
        Ref<ComputeBuffer> PropertyIndices;
        uint TotalDrawCallCount;
    };

    struct MeshBatchCollection
    {
        std::vector<MeshOnlyBatch> MeshBatches;
        std::unordered_map<ulong, uint> BatchMap;
        Ref<ComputeBuffer> MatrixBuffer;
        uint TotalDrawCallCount;
    };

    void ResetCollection(DynamicBatchCollection* collection);
    void ResetCollection(MeshBatchCollection* collection);
    
    void QueueDraw(DynamicBatchCollection* collection, Weak<Mesh>& mesh, int submesh, Weak<Material>& material, float4x4* localToWorld, float4x4* worldToLocal);
    void QueueDraw(MeshBatchCollection* collection, Weak<Mesh>& mesh, float4x4* localToWorld, float4x4* worldToLocal);

    void UpdateBuffers(DynamicBatchCollection* collection);
    void UpdateBuffers(MeshBatchCollection* collection);

    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex);
    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial);
    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader);
    
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial);
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader);
}