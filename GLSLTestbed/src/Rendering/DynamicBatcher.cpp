#include "PrecompiledHeader.h"
#include "Utilities/Utilities.h"
#include "Utilities/HashCache.h"
#include "Rendering/DynamicBatcher.h"
#include "Rendering/Graphics.h"

namespace PK::Rendering
{
    using namespace Utilities;
    using namespace Objects;

    void DynamicBatcher::Reset()
    {
        m_batchedMatrixCount = 0;

        for (auto& batch : m_meshBatches)
        {
            batch.count = 0;
            batch.instancingOffset = 0;
            batch.materialBatchCount = 0;
        }

        for (auto& batch : m_materialBatches)
        {
            batch.count = 0;
            batch.instancingOffset = 0;
        }
    }
    
    void DynamicBatcher::QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, float4x4* localToWorld, float4x4* worldToLocal)
    {
        auto materialId = material.lock()->GetAssetID();
        auto meshId = mesh.lock()->GetGraphicsID();
        auto submeshKey = ((uint)submesh << 24) | ((uint)materialId & 0xFFFFFF);
        auto batchKey = (ulong)meshId << 32 | ((ulong)(uint)submeshKey & 0xFFFFFFFF);

        uint materialBatchIndex = 0;
        MeshBatch* meshBatch = nullptr;
        MaterialBatch* materialBatch = nullptr;

        if (m_meshBatchMap.count(meshId) < 1)
        {
            auto meshBatchIndex = (uint)m_meshBatches.size();
            m_meshBatchMap[meshId] = meshBatchIndex;
            m_meshBatches.push_back(MeshBatch());
            meshBatch = &m_meshBatches.at(meshBatchIndex);
            meshBatch->mesh = mesh;
        }
        else
        {
            meshBatch = &m_meshBatches.at(m_meshBatchMap.at(meshId));
        }
        
        if (m_materialBatchMap.count(batchKey) < 1)
        {
            materialBatchIndex = (uint)m_materialBatches.size();
            m_materialBatchMap[batchKey] = materialBatchIndex;

            m_materialBatches.push_back(MaterialBatch());
            materialBatch = &m_materialBatches.at(materialBatchIndex);
            materialBatch->material = material;
            materialBatch->submesh = submesh;
        }
        else
        {
            materialBatchIndex = m_materialBatchMap.at(batchKey);
            materialBatch = &m_materialBatches.at(materialBatchIndex);
        }

        if (materialBatch->count == 0)
        {
            Utilities::ValidateVectorSize(meshBatch->materialBatches, meshBatch->materialBatchCount + 1);
            meshBatch->materialBatches[meshBatch->materialBatchCount++] = materialBatchIndex;
        }

        Utilities::ValidateVectorSize(materialBatch->matrices, (materialBatch->count + 1) * 2);
        materialBatch->matrices[materialBatch->count * 2 + 0] = localToWorld;
        materialBatch->matrices[materialBatch->count * 2 + 1] = worldToLocal;
        ++materialBatch->count;
        ++meshBatch->count;
        m_batchedMatrixCount += 2;
    }
    
    void DynamicBatcher::UpdateBuffers()
    {
        if (m_batchedMatrixCount < 1)
        {
            return;
        }

        if (m_matrixBuffer == nullptr)
        {
            m_matrixBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)m_batchedMatrixCount, GL_STATIC_DRAW);
        }
        else
        {
            m_matrixBuffer->ValidateSize((uint)m_batchedMatrixCount);
        }

        auto buffer = m_matrixBuffer->BeginMapBufferRange<float4x4>(0, m_batchedMatrixCount);
        auto offset = 0;

        for (auto& meshBatch : m_meshBatches)
        {
            if (meshBatch.count < 1)
            {
                continue;
            }

            meshBatch.instancingOffset = offset;
            auto materialBatches = meshBatch.materialBatches.data();

            for (uint i = 0; i < meshBatch.materialBatchCount; ++i)
            {
                auto* materialBatch = &m_materialBatches.at(materialBatches[i]);
                materialBatch->instancingOffset = offset;

                float4x4** matrices = materialBatch->matrices.data();
                auto matrixcount = materialBatch->count * 2;
                auto bufferOffset = offset * 2;

                for (uint i = 0; i < matrixcount; ++i)
                {
                    buffer[bufferOffset + i] = *matrices[i];
                }

                offset += materialBatch->count;
            }
        }

        m_matrixBuffer->EndMapBuffer();
    }
    
    void DynamicBatcher::Execute(uint32_t renderQueueIndex)
    {
        auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
        auto pk_InstancingOffset = StringHashID::StringToID("pk_InstancingOffset");
        GraphicsAPI::SetGlobalComputeBuffer(HashCache::Get()->pk_InstancingMatrices, m_matrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
    
        for (auto& meshBatch : m_meshBatches)
        {
            auto materialBatches = meshBatch.materialBatches.data();
            auto mesh = meshBatch.mesh.lock();

            for (uint i = 0; i < meshBatch.materialBatchCount; ++i)
            {
                auto* materialBatch = &m_materialBatches.at(materialBatches[i]);
                GraphicsAPI::SetGlobalUInt(pk_InstancingOffset, materialBatch->instancingOffset);
                GraphicsAPI::DrawMeshInstanced(mesh, materialBatch->submesh, (uint)materialBatch->count, materialBatch->material.lock());
            }
        }

        GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
    }
    
    void DynamicBatcher::Execute(uint32_t renderQueueIndex, Ref<Material>& overrideMaterial)
    {
        auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
        auto pk_InstancingOffset = StringHashID::StringToID("pk_InstancingOffset");
        GraphicsAPI::SetGlobalComputeBuffer(HashCache::Get()->pk_InstancingMatrices, m_matrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
    
        for (auto& meshBatch : m_meshBatches)
        {
            if (meshBatch.count < 1)
            {
                continue;
            }

            auto mesh = meshBatch.mesh.lock();
            GraphicsAPI::SetGlobalUInt(pk_InstancingOffset, meshBatch.instancingOffset);
            GraphicsAPI::DrawMeshInstanced(mesh, -1, (uint)meshBatch.count, overrideMaterial);
        }

        GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
    }

    void DynamicBatcher::Execute(uint32_t renderQueueIndex, Ref<Shader>& overrideShader)
    {
        auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
        auto pk_InstancingOffset = StringHashID::StringToID("pk_InstancingOffset");
        GraphicsAPI::SetGlobalComputeBuffer(HashCache::Get()->pk_InstancingMatrices, m_matrixBuffer->GetGraphicsID());
        GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);

        for (auto& meshBatch : m_meshBatches)
        {
            if (meshBatch.count < 1)
            {
                continue;
            }

            auto mesh = meshBatch.mesh.lock();
            GraphicsAPI::SetGlobalUInt(pk_InstancingOffset, meshBatch.instancingOffset);
            GraphicsAPI::DrawMeshInstanced(mesh, -1, (uint)meshBatch.count, overrideShader);
        }

        GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
    }
}