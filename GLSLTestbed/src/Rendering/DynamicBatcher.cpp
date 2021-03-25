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
        for (auto& batch : m_batches)
        {
            batch.count = 0;
        }
    }
    
    void DynamicBatcher::QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, float4x4* localToWorld, float4x4* worldToLocal)
    {
        auto materialId = material.lock()->GetAssetID();
        auto meshId = mesh.lock()->GetGraphicsID();
        auto submeshKey = ((uint)submesh << 24) | ((uint)materialId & 0xFFFFFF);
        auto batchKey = (ulong)meshId << 32 | ((ulong)(uint)submeshKey & 0xFFFFFFFF);

        DynamicBatch* batch = nullptr;
    
        if (m_batchmap.count(batchKey))
        {
            batch = &m_batches.at(m_batchmap.at(batchKey));
        }
        else
        {
            auto index = (uint)m_batches.size();
            m_batchmap[batchKey] = index;
            m_batches.push_back(DynamicBatch());
            batch = &m_batches.at(index);
            batch->material = material;
            batch->mesh = mesh;
        }
    
        Utilities::ValidateVectorSize(batch->matrices, (batch->count + 1) * 2);
        batch->matrices[batch->count * 2 + 0] = localToWorld;
        batch->matrices[batch->count * 2 + 1] = worldToLocal;
        ++batch->count;
    }
    
    void DynamicBatcher::UpdateBuffers()
    {
        for (auto& batch : m_batches)
        {
            if (batch.count <= 1)
            {
                continue;
            }

            if (batch.instancingBuffer == nullptr)
            {
                batch.instancingBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE::FLOAT4X4, "Matrix"} }), (uint)(batch.count * 2));
            }
            else
            {
                batch.instancingBuffer->ValidateSize((uint)batch.count * 2);
            }
    
            auto buffer = batch.instancingBuffer->BeginMapBufferRange<float4x4>(0, batch.count * 2);
            auto matrices = batch.matrices.data();

            for (uint i = 0; i < buffer.count; ++i)
            {
                buffer[i] = *matrices[i];
            }

            batch.instancingBuffer->EndMapBuffer();
        }
    }
    
    void DynamicBatcher::Execute(uint32_t renderQueueIndex)
    {
        auto pk_InstancingData = HashCache::Get()->pk_InstancingMatrices;
        auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
    
        for (auto& batch : m_batches)
        {
            if (batch.count < 1)
            {
                continue;
            }
    
            if (batch.count > 1)
            {
                GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
                GraphicsAPI::SetGlobalComputeBuffer(pk_InstancingData, batch.instancingBuffer->GetGraphicsID());
                GraphicsAPI::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, (uint)batch.count, batch.material.lock());
                GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
                continue;
            }
    
            GraphicsAPI::DrawMesh(batch.mesh.lock(), batch.submesh, batch.material.lock(), *batch.matrices[0], *batch.matrices[1]);
        }
    }
    
    void DynamicBatcher::Execute(uint32_t renderQueueIndex, Ref<Material>& overrideMaterial)
    {
        auto pk_InstancingData = HashCache::Get()->pk_InstancingMatrices;
        auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
    
        for (auto& batch : m_batches)
        {
            if (batch.count < 1)
            {
                continue;
            }
    
            if (batch.count > 1)
            {
                GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
                GraphicsAPI::SetGlobalComputeBuffer(pk_InstancingData, batch.instancingBuffer->GetGraphicsID());
                GraphicsAPI::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, (uint)batch.count, overrideMaterial);
                GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
                continue;
            }

            GraphicsAPI::DrawMesh(batch.mesh.lock(), batch.submesh, overrideMaterial, *batch.matrices[0], *batch.matrices[1]);
        }
    }

    void DynamicBatcher::Execute(uint32_t renderQueueIndex, Ref<Shader>& overrideShader)
    {
        auto pk_InstancingData = HashCache::Get()->pk_InstancingMatrices;
        auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;

        for (auto& batch : m_batches)
        {
            if (batch.count < 1)
            {
                continue;
            }

            if (batch.count > 1)
            {
                GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
                GraphicsAPI::SetGlobalComputeBuffer(pk_InstancingData, batch.instancingBuffer->GetGraphicsID());
                GraphicsAPI::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, (uint)batch.count, overrideShader);
                GraphicsAPI::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
                continue;
            }

            GraphicsAPI::DrawMesh(batch.mesh.lock(), batch.submesh, overrideShader, *batch.matrices[0], *batch.matrices[1]);
        }
    }
}