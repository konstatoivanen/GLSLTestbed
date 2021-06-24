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
        float4x4* localToWorld = nullptr;
        float depth = 0.0f;
    };

    struct DrawcallIndexed
    {
        float4x4* localToWorld = nullptr;
        float depth = 0.0f;
        uint index = 0;
    };

    struct BatchBase
    {
        uint instancingOffset = 0;
        uint drawCallCount = 0;
    };

    struct MaterialBatch : BatchBase
    {
        const Material* material = nullptr;
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
        const Mesh* mesh = nullptr;
        std::vector<uint> shaderBatches;
        uint shaderBatchCount = 0;
    };

    struct MeshOnlyBatch : BatchBase
    {
        const Mesh* mesh = nullptr;
        std::vector<Drawcall> drawcalls;
    };

    struct IndexedMeshBatch : BatchBase
    {
        const Mesh* mesh = nullptr;
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
        uint TotalDrawCallCount = 0;
    };

    struct MeshBatchCollection
    {
        std::vector<MeshOnlyBatch> MeshBatches;
        std::unordered_map<ulong, uint> BatchMap;
        Ref<ComputeBuffer> MatrixBuffer;
        uint TotalDrawCallCount = 0;
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

    void DrawBatches(DynamicBatchCollection* collection);
    void DrawBatches(DynamicBatchCollection* collection, const Material* overrideMaterial);
    void DrawBatches(DynamicBatchCollection* collection, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(DynamicBatchCollection* collection, Shader* overrideShader);

    void DrawBatchesPredicated(DynamicBatchCollection* collection, const uint32_t keyword, Shader* fallbackShader, const FixedStateAttributes& attributes);
    
    void DrawBatches(MeshBatchCollection* collection, const Material* overrideMaterial);
    void DrawBatches(MeshBatchCollection* collection, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(MeshBatchCollection* collection, Shader* overrideShader);

    void DrawBatches(IndexedMeshBatchCollection* collection, const Material* overrideMaterial);
    void DrawBatches(IndexedMeshBatchCollection* collection, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock);
    void DrawBatches(IndexedMeshBatchCollection* collection, Shader* overrideShader);
}