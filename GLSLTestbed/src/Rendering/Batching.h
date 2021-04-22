#pragma once
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Mesh.h"

namespace PK::Rendering::Batching
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;
    
    struct Drawcall
    {
        float4x4* localToWorld;
        float depth;
    };

    struct DrawcallIndexed
    {
        float4x4* localToWorld;
        float depth;
        uint index;
    };

    struct BatchBase
    {
        uint instancingOffset;
        uint drawCallCount;
    };

    struct MaterialBatch : BatchBase
    {
        const Material* material;
        std::vector<Drawcall> drawcalls;
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
        const Mesh* mesh;
        std::vector<uint> shaderBatches;
        uint shaderBatchCount = 0;
    };

    struct MeshOnlyBatch : BatchBase
    {
        const Mesh* mesh;
        std::vector<Drawcall> drawcalls;
    };

    struct IndexedMeshBatch : BatchBase
    {
        const Mesh* mesh;
        std::vector<DrawcallIndexed> drawcalls;
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

    struct IndexedMeshBatchCollection
    {
        std::vector<IndexedMeshBatch> MeshBatches;
        std::unordered_map<ulong, uint> BatchMap;
        Ref<ComputeBuffer> MatrixBuffer;
        Ref<ComputeBuffer> IndexBuffer;
        uint TotalDrawCallCount = 0;
    };

    void ResetCollection(DynamicBatchCollection* collection);
    void ResetCollection(MeshBatchCollection* collection);
    void ResetCollection(IndexedMeshBatchCollection* collection);
    
    void QueueDraw(DynamicBatchCollection* collection, const Mesh* mesh, int submesh, const Material* material, const Drawcall& drawcall);
    void QueueDraw(MeshBatchCollection* collection, const Mesh* mesh, const Drawcall& drawcall);
    void QueueDraw(IndexedMeshBatchCollection* collection, const Mesh* mesh, const DrawcallIndexed& drawcall);

    void UpdateBuffers(DynamicBatchCollection* collection);
    void UpdateBuffers(MeshBatchCollection* collection);
    void UpdateBuffers(IndexedMeshBatchCollection* collection);

    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex);
    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial);
    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader);
    
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial);
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader);

    void DrawBatches(IndexedMeshBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial);
    void DrawBatches(IndexedMeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(IndexedMeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader);
}