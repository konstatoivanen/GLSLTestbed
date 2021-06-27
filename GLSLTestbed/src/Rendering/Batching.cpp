#include "PrecompiledHeader.h"
#include "Utilities/Utilities.h"
#include "Utilities/HashCache.h"
#include "Rendering/Batching.h"
#include "Rendering/GraphicsAPI.h"

namespace PK::Rendering::Batching
{
    using namespace Utilities;
    using namespace Objects;

    template<typename T>
    static bool GetBatch(std::unordered_map<ulong, uint>& batchmap, std::vector<T>& batches, ulong key, T** batch, uint* batchIndex)
    {
        if (batchmap.count(key) < 1)
        {
            *batchIndex = (uint)batches.size();
            batchmap[key] = *batchIndex;
            batches.push_back(T());
            *batch = &batches.at(*batchIndex);
            return true;
        }
        else
        {
            *batchIndex = batchmap.at(key);
            *batch = &batches.at(*batchIndex);
            return false;
        }
    }

    void ResetCollection(DynamicBatchCollection* collection)
    {
        collection->TotalDrawCallCount = 0;

        for (auto& batch : collection->MeshBatches)
        {
            batch.drawCallCount = 0;
            batch.instancingOffset = 0;
            batch.shaderBatchCount = 0;
        }

        for (auto& batch : collection->MaterialBatches)
        {
            batch.drawCallCount = 0;
            batch.instancingOffset = 0;
        }

        for (auto& batch : collection->ShaderBatches)
        {
            batch.drawCallCount = 0;
            batch.materialBatchCount = 0;
            batch.instancingOffset = 0;
        }
    }

    void ResetCollection(MeshBatchCollection* collection)
    {
        collection->TotalDrawCallCount = 0;

        for (auto& batch : collection->MeshBatches)
        {
            batch.drawCallCount = 0;
            batch.instancingOffset = 0;
        }
    }

    void ResetCollection(IndexedMeshBatchCollection* collection)
    {
        collection->TotalDrawCallCount = 0;

        for (auto& batch : collection->MeshBatches)
        {
            batch.drawCallCount = 0;
            batch.instancingOffset = 0;
        }
    }

  
    void QueueDraw(DynamicBatchCollection* collection, const Mesh* mesh, int submesh, const Material* material, const Drawcall& drawcall)
    {
        auto meshId = (ulong)mesh->GetGraphicsID();
        auto materialId = (ulong)material->GetAssetID();
        auto shaderId = (ulong)material->GetShaderAssetID();

        auto meshKey = (ulong)(meshId & 0xFFFF);
        auto submeshKey = (ulong)(((ulong)submesh << 16ul) | meshKey);
        auto shaderKey = (ulong)((shaderId << 32ul) | submeshKey);
        auto materialKey = (ulong)((materialId << 48ul) | shaderKey);

        uint meshBatchIndex = 0;
        uint materialBatchIndex = 0;
        uint shaderBatchIndex = 0;
        MeshBatch* meshBatch = nullptr;
        ShaderBatch* shaderBatch = nullptr;
        MaterialBatch* materialBatch = nullptr;

        GetBatch(collection->BatchMap, collection->MeshBatches, meshKey, &meshBatch, &meshBatchIndex);
        meshBatch->mesh = mesh;

        if (GetBatch(collection->BatchMap, collection->ShaderBatches, shaderKey, &shaderBatch, &shaderBatchIndex))
        {
            auto& instancingInfo = material->GetShader()->GetInstancingInfo();
            shaderBatch->submesh = submesh;

            if (instancingInfo.hasInstancedProperties)
            {
                shaderBatch->instancedData = CreateRef<ComputeBuffer>(instancingInfo.propertyLayout, 1, false, GL_STREAM_DRAW);
            }
        }

        GetBatch(collection->BatchMap, collection->MaterialBatches, materialKey, &materialBatch, &materialBatchIndex);
        materialBatch->material = material;

        if (shaderBatch->drawCallCount == 0)
        {
            Utilities::PushVectorElementRef(meshBatch->shaderBatches, &meshBatch->shaderBatchCount, shaderBatchIndex);
        }

        if (materialBatch->drawCallCount == 0)
        {
            Utilities::PushVectorElementRef(shaderBatch->materialBatches, &shaderBatch->materialBatchCount, materialBatchIndex);
        }

        Utilities::ValidateVectorSize(materialBatch->drawcalls, materialBatch->drawCallCount + 1);
        materialBatch->drawcalls[materialBatch->drawCallCount++] = drawcall;
        ++shaderBatch->drawCallCount;
        ++meshBatch->drawCallCount;
        ++collection->TotalDrawCallCount;
    }

    void QueueDraw(MeshBatchCollection* collection, const Mesh* mesh, const Drawcall& drawcall)
    {
        auto meshId = (ulong)mesh->GetGraphicsID();
        
        uint meshBatchIndex = 0;
        MeshOnlyBatch* meshBatch = nullptr;

        GetBatch(collection->BatchMap, collection->MeshBatches, meshId, &meshBatch, &meshBatchIndex);
        meshBatch->mesh = mesh;

        Utilities::ValidateVectorSize(meshBatch->drawcalls, meshBatch->drawCallCount + 1);
        meshBatch->drawcalls[meshBatch->drawCallCount++] = drawcall;
        ++collection->TotalDrawCallCount;
    }

    void QueueDraw(IndexedMeshBatchCollection* collection, const Mesh* mesh, const DrawcallIndexed& drawcall)
    {
        auto meshId = (ulong)mesh->GetGraphicsID();

        uint meshBatchIndex = 0;
        IndexedMeshBatch* meshBatch = nullptr;

        GetBatch(collection->BatchMap, collection->MeshBatches, meshId, &meshBatch, &meshBatchIndex);
        meshBatch->mesh = mesh;

        Utilities::ValidateVectorSize(meshBatch->drawcalls, meshBatch->drawCallCount + 1);
        meshBatch->drawcalls[meshBatch->drawCallCount++] = drawcall;
        ++collection->TotalDrawCallCount;
    }

   
    void UpdateBuffers(DynamicBatchCollection* collection)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        if (collection->PropertyIndices == nullptr)
        {
            collection->PropertyIndices = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::UINT, "Index"} }), (uint)collection->TotalDrawCallCount, false, GL_STREAM_DRAW);
        }
        else
        {
            collection->PropertyIndices->ValidateSize((uint)collection->TotalDrawCallCount);
        }

        if (collection->MatrixBuffer == nullptr)
        {
            collection->MatrixBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)collection->TotalDrawCallCount, false, GL_STREAM_DRAW);
        }
        else
        {
            collection->MatrixBuffer->ValidateSize((uint)collection->TotalDrawCallCount);
        }

        auto indexBuffer = collection->PropertyIndices->BeginMapBufferRange<uint>(0, collection->TotalDrawCallCount);
        auto matrixBuffer = collection->MatrixBuffer->BeginMapBufferRange<float4x4>(0, collection->TotalDrawCallCount);
        auto shaderBatches = collection->ShaderBatches.data();
        auto materialBatches = collection->MaterialBatches.data();
        char* instancedDataBuffer = nullptr;
        const BufferLayout* instancingLayout = nullptr;
        size_t stride = 0;
        size_t offset = 0;

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            meshBatch.instancingOffset = (uint)offset;
            auto shaderBatchIndices = meshBatch.shaderBatches.data();

            for (uint i = 0; i < meshBatch.shaderBatchCount; ++i)
            {
                auto* shaderBatch = &shaderBatches[shaderBatchIndices[i]];

                if (shaderBatch->instancedData != nullptr)
                {
                    stride = shaderBatch->instancedData->GetStride();
                    instancingLayout = &shaderBatch->instancedData->GetLayout();
                    shaderBatch->instancingOffset = (uint)offset;
                    shaderBatch->instancedData->ValidateSize(shaderBatch->drawCallCount);
                    instancedDataBuffer = shaderBatch->instancedData->BeginMapBufferRange<char>(0, shaderBatch->drawCallCount * stride).data;
                }

                auto materialBatchIndices = shaderBatch->materialBatches.data();

                for (uint j = 0; j < shaderBatch->materialBatchCount; ++j)
                {
                    auto* materialBatch = &materialBatches[materialBatchIndices[j]];
                    materialBatch->instancingOffset = (uint)offset;

                    if (shaderBatch->instancedData != nullptr)
                    {
                        materialBatch->material->CopyBufferLayout(*instancingLayout, instancedDataBuffer + j * stride);
                    }

                    Drawcall* drawcalls = materialBatch->drawcalls.data();

                    for (uint k = 0; k < materialBatch->drawCallCount; ++k)
                    {
                        indexBuffer[offset + k] = j;
                        matrixBuffer[offset + k] = *drawcalls[k].localToWorld;
                    }

                    offset += materialBatch->drawCallCount;
                }

                if (shaderBatch->instancedData != nullptr)
                {
                    shaderBatch->instancedData->EndMapBuffer();
                }
            }
        }

        collection->PropertyIndices->EndMapBuffer();
        collection->MatrixBuffer->EndMapBuffer();
    }

    void UpdateBuffers(MeshBatchCollection* collection)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        if (collection->MatrixBuffer == nullptr)
        {
            collection->MatrixBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)collection->TotalDrawCallCount, false, GL_STREAM_DRAW);
        }
        else
        {
            collection->MatrixBuffer->ValidateSize((uint)collection->TotalDrawCallCount);
        }

        auto matrixBuffer = collection->MatrixBuffer->BeginMapBufferRange<float4x4>(0, collection->TotalDrawCallCount);
        size_t offset = 0;

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            meshBatch.instancingOffset = (uint)offset;
            Drawcall* drawcalls = meshBatch.drawcalls.data();

            for (uint i = 0; i < meshBatch.drawCallCount; ++i)
            {
                matrixBuffer[offset + i] = *drawcalls[i].localToWorld;
            }

            offset += meshBatch.drawCallCount;
        }

        collection->MatrixBuffer->EndMapBuffer();
    }

    void UpdateBuffers(IndexedMeshBatchCollection* collection)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        if (collection->MatrixBuffer == nullptr)
        {
            collection->MatrixBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)collection->TotalDrawCallCount, false, GL_STREAM_DRAW);
        }
        else
        {
            collection->MatrixBuffer->ValidateSize((uint)collection->TotalDrawCallCount);
        }

        if (collection->IndexBuffer == nullptr)
        {
            collection->IndexBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::UINT, "Index"} }), (uint)collection->TotalDrawCallCount, false, GL_STREAM_DRAW);
        }
        else
        {
            collection->IndexBuffer->ValidateSize((uint)collection->TotalDrawCallCount);
        }


        auto matrixBuffer = collection->MatrixBuffer->BeginMapBufferRange<float4x4>(0, collection->TotalDrawCallCount);
        auto indexBuffer = collection->IndexBuffer->BeginMapBufferRange<uint>(0, collection->TotalDrawCallCount);
        size_t offset = 0;

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            auto* drawcalls = meshBatch.drawcalls.data();
            meshBatch.instancingOffset = (uint)offset;

            for (uint i = 0; i < meshBatch.drawCallCount; ++i)
            {
                indexBuffer[offset + i] = drawcalls[i].index;
                matrixBuffer[offset + i] = *drawcalls[i].localToWorld;
            }

            offset += meshBatch.drawCallCount;
        }

        collection->MatrixBuffer->EndMapBuffer();
        collection->IndexBuffer->EndMapBuffer();
    }


    void DrawBatches(DynamicBatchCollection* collection)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingPropertyIndices, collection->PropertyIndices->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            auto shaderBatches = meshBatch.shaderBatches.data();

            for (uint i = 0; i < meshBatch.shaderBatchCount; ++i)
            {
                auto* shaderBatch = &collection->ShaderBatches.at(shaderBatches[i]);
                auto  instancedData = shaderBatch->instancedData.get();

                if (instancedData != nullptr)
                {
                    auto* firstMaterial = &collection->MaterialBatches.at(shaderBatch->materialBatches.at(0));
                    GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancedProperties, instancedData->GetGraphicsID());
                    GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, shaderBatch->submesh, shaderBatch->instancingOffset, (uint)shaderBatch->drawCallCount, firstMaterial->material);
                }
                else
                {
                    auto materialBatchIndices = shaderBatch->materialBatches.data();

                    for (uint j = 0; j < shaderBatch->materialBatchCount; ++j)
                    {
                        auto* materialBatch = &collection->MaterialBatches.at(materialBatchIndices[j]);
                        GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, shaderBatch->submesh, materialBatch->instancingOffset, (uint)materialBatch->drawCallCount, materialBatch->material);
                    }
                }
            }
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(DynamicBatchCollection* collection, const Material* overrideMaterial)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideMaterial);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(DynamicBatchCollection* collection, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideShader, propertyBlock);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(DynamicBatchCollection* collection, Shader* overrideShader)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideShader);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatchesPredicated(DynamicBatchCollection* collection, const uint32_t keyword, Shader* fallbackShader, const FixedStateAttributes& attributes)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingPropertyIndices, collection->PropertyIndices->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);
        GraphicsAPI::SetGlobalKeyword(keyword, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            auto shaderBatches = meshBatch.shaderBatches.data();

            for (uint i = 0; i < meshBatch.shaderBatchCount; ++i)
            {
                auto* shaderBatch = &collection->ShaderBatches.at(shaderBatches[i]);
                auto  instancedData = shaderBatch->instancedData.get();
                auto* firstMaterial = &collection->MaterialBatches.at(shaderBatch->materialBatches.at(0));

                if (!firstMaterial->material->SupportsKeyword(keyword))
                {
                    GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, shaderBatch->instancingOffset, (uint)shaderBatch->drawCallCount, fallbackShader, attributes);
                    continue;
                }

                if (instancedData != nullptr)
                {
                    GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancedProperties, instancedData->GetGraphicsID());
                    GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, shaderBatch->submesh, shaderBatch->instancingOffset, (uint)shaderBatch->drawCallCount, firstMaterial->material, attributes);
                }
                else
                {
                    auto materialBatchIndices = shaderBatch->materialBatches.data();

                    for (uint j = 0; j < shaderBatch->materialBatchCount; ++j)
                    {
                        auto* materialBatch = &collection->MaterialBatches.at(materialBatchIndices[j]);
                        GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, shaderBatch->submesh, materialBatch->instancingOffset, (uint)materialBatch->drawCallCount, materialBatch->material, attributes);
                    }
                }
            }
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
        GraphicsAPI::SetGlobalKeyword(keyword, false);
    }

    void DrawBatchesPredicated(DynamicBatchCollection* collection, const uint32_t keyword, Shader* fallbackShader, const ShaderPropertyBlock& propertyBlock, const FixedStateAttributes& attributes)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingPropertyIndices, collection->PropertyIndices->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);
        GraphicsAPI::SetGlobalKeyword(keyword, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            auto shaderBatches = meshBatch.shaderBatches.data();

            for (uint i = 0; i < meshBatch.shaderBatchCount; ++i)
            {
                auto* shaderBatch = &collection->ShaderBatches.at(shaderBatches[i]);
                auto  instancedData = shaderBatch->instancedData.get();
                auto* firstMaterial = &collection->MaterialBatches.at(shaderBatch->materialBatches.at(0));

                if (!firstMaterial->material->SupportsKeyword(keyword))
                {
                    GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, shaderBatch->instancingOffset, (uint)shaderBatch->drawCallCount, fallbackShader, propertyBlock, attributes);
                    continue;
                }

                if (instancedData != nullptr)
                {
                    GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancedProperties, instancedData->GetGraphicsID());
                    GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, shaderBatch->submesh, shaderBatch->instancingOffset, (uint)shaderBatch->drawCallCount, firstMaterial->material, propertyBlock, attributes);
                }
                else
                {
                    auto materialBatchIndices = shaderBatch->materialBatches.data();

                    for (uint j = 0; j < shaderBatch->materialBatchCount; ++j)
                    {
                        auto* materialBatch = &collection->MaterialBatches.at(materialBatchIndices[j]);
                        GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, shaderBatch->submesh, materialBatch->instancingOffset, (uint)materialBatch->drawCallCount, materialBatch->material, propertyBlock, attributes);
                    }
                }
            }
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
        GraphicsAPI::SetGlobalKeyword(keyword, false);
    }

   
    void DrawBatches(MeshBatchCollection* collection, const Material* overrideMaterial)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideMaterial);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(MeshBatchCollection* collection, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideShader, propertyBlock);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(MeshBatchCollection* collection, Shader* overrideShader)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideShader);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(IndexedMeshBatchCollection* collection, const Material* overrideMaterial)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingPropertyIndices, collection->IndexBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideMaterial);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(IndexedMeshBatchCollection* collection, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingPropertyIndices, collection->IndexBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideShader, propertyBlock);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }

    void DrawBatches(IndexedMeshBatchCollection* collection, Shader* overrideShader)
    {
        if (collection->TotalDrawCallCount < 1)
        {
            return;
        }

        auto hashes = HashCache::Get();
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingMatrices, collection->MatrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalComputeBuffer(hashes->pk_InstancingPropertyIndices, collection->IndexBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            GraphicsAPI::DrawMeshInstanced(meshBatch.mesh, -1, meshBatch.instancingOffset, (uint)meshBatch.drawCallCount, overrideShader);
        }

        GraphicsAPI::SetGlobalKeyword(hashes->PK_ENABLE_INSTANCING, false);
    }
}