#include "PrecompiledHeader.h"
#include "Utilities/Utilities.h"
#include "Utilities/HashCache.h"
#include "Rendering/Batching.h"
#include "Rendering/Graphics.h"

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

  
    void QueueDraw(DynamicBatchCollection* collection, Weak<Mesh>& mesh, int submesh, Weak<Material>& material, float4x4* localToWorld, float4x4* worldToLocal)
    {
        auto meshref = mesh.lock();
        auto materialref = material.lock();

        auto meshId = (ulong)meshref->GetGraphicsID();
        auto materialId = (ulong)materialref->GetAssetID();
        auto shaderId = (ulong)materialref->GetShaderAssetID();

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
        meshBatch->mesh = meshref.get();

        if (GetBatch(collection->BatchMap, collection->ShaderBatches, shaderKey, &shaderBatch, &shaderBatchIndex))
        {
            auto& instancingInfo = materialref->GetShader().lock()->GetInstancingInfo();
            shaderBatch->submesh = submesh;

            if (instancingInfo.hasInstancedProperties)
            {
                shaderBatch->instancedData = CreateRef<ComputeBuffer>(instancingInfo.propertyLayout, 1, GL_STREAM_DRAW);
            }
        }

        GetBatch(collection->BatchMap, collection->MaterialBatches, materialKey, &materialBatch, &materialBatchIndex);
        materialBatch->material = materialref.get();

        if (shaderBatch->drawCallCount == 0)
        {
            Utilities::PushVectorElementRef(meshBatch->shaderBatches, &meshBatch->shaderBatchCount, shaderBatchIndex);
        }

        if (materialBatch->drawCallCount == 0)
        {
            Utilities::PushVectorElementRef(shaderBatch->materialBatches, &shaderBatch->materialBatchCount, materialBatchIndex);
        }

        Utilities::ValidateVectorSize(materialBatch->matrices, (materialBatch->drawCallCount + 1) * 2);
        materialBatch->matrices[materialBatch->drawCallCount * 2 + 0] = localToWorld;
        materialBatch->matrices[materialBatch->drawCallCount * 2 + 1] = worldToLocal;
        ++materialBatch->drawCallCount;
        ++shaderBatch->drawCallCount;
        ++meshBatch->drawCallCount;
        ++collection->TotalDrawCallCount;
    }

    void QueueDraw(MeshBatchCollection* collection, Weak<Mesh>& mesh, float4x4* localToWorld, float4x4* worldToLocal)
    {
        auto meshref = mesh.lock();
        auto meshId = (ulong)meshref->GetGraphicsID();
        
        uint meshBatchIndex = 0;
        MeshOnlyBatch* meshBatch = nullptr;

        GetBatch(collection->BatchMap, collection->MeshBatches, meshId, &meshBatch, &meshBatchIndex);
        meshBatch->mesh = meshref.get();

        Utilities::ValidateVectorSize(meshBatch->matrices, (meshBatch->drawCallCount + 1) * 2);
        meshBatch->matrices[meshBatch->drawCallCount * 2 + 0] = localToWorld;
        meshBatch->matrices[meshBatch->drawCallCount * 2 + 1] = worldToLocal;
        ++meshBatch->drawCallCount;
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
            collection->PropertyIndices = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::UINT, "Index"} }), (uint)collection->TotalDrawCallCount, GL_STREAM_DRAW);
        }
        else
        {
            collection->PropertyIndices->ValidateSize((uint)collection->TotalDrawCallCount);
        }

        if (collection->MatrixBuffer == nullptr)
        {
            collection->MatrixBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)collection->TotalDrawCallCount * 2, GL_STREAM_DRAW);
        }
        else
        {
            collection->MatrixBuffer->ValidateSize((uint)collection->TotalDrawCallCount * 2);
        }

        auto indexBuffer = collection->PropertyIndices->BeginMapBufferRange<uint>(0, collection->TotalDrawCallCount);
        auto matrixBuffer = collection->MatrixBuffer->BeginMapBufferRange<float4x4>(0, collection->TotalDrawCallCount * 2);
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

                    float4x4** matrices = materialBatch->matrices.data();
                    auto matrixOffset = offset * 2;

                    for (uint k = 0, l = 0; k < materialBatch->drawCallCount; ++k, l += 2)
                    {
                        indexBuffer[offset + k] = j;
                        matrixBuffer[matrixOffset + l + 0] = *matrices[l + 0];
                        matrixBuffer[matrixOffset + l + 1] = *matrices[l + 1];
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
            collection->MatrixBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)collection->TotalDrawCallCount * 2, GL_STREAM_DRAW);
        }
        else
        {
            collection->MatrixBuffer->ValidateSize((uint)collection->TotalDrawCallCount * 2);
        }

        auto matrixBuffer = collection->MatrixBuffer->BeginMapBufferRange<float4x4>(0, collection->TotalDrawCallCount * 2);
        size_t offset = 0;

        for (auto& meshBatch : collection->MeshBatches)
        {
            if (meshBatch.drawCallCount < 1)
            {
                continue;
            }

            meshBatch.instancingOffset = (uint)offset;
            float4x4** matrices = meshBatch.matrices.data();
            auto matrixOffset = offset * 2;
            auto count = meshBatch.drawCallCount * 2;

            for (uint i = 0; i < count; ++i)
            {
                matrixBuffer[matrixOffset + i] = *matrices[i];
            }

            offset += meshBatch.drawCallCount;
        }

        collection->MatrixBuffer->EndMapBuffer();
    }


    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex)
    {
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

    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial)
    {
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

    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock)
    {
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

    void DrawBatches(DynamicBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader)
    {
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

   
    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, const Material* overrideMaterial)
    {
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

    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader, const ShaderPropertyBlock& propertyBlock)
    {
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

    void DrawBatches(MeshBatchCollection* collection, uint32_t renderQueueIndex, Shader* overrideShader)
    {
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
}