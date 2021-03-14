#include "PrecompiledHeader.h"
#include "Utilities/HashCache.h"
#include "Rendering/DynamicBatcher.h"
#include "Rendering/Graphics.h"

void DynamicBatcher::Reset()
{
    for (auto& batch : m_batches)
    {
        batch.count = 0;
    }
}

void DynamicBatcher::QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, const float4x4& matrix)
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

    CGMath::ValidateVectorSize(batch->matrices, batch->count + 1);
    batch->matrices[batch->count++] = matrix;
}

void DynamicBatcher::UpdateBuffers()
{
    for (auto& batch : m_batches)
    {
        if (batch.instancingBuffer == nullptr)
        {
            batch.instancingBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE_FLOAT4X4, "Matrix" } }), batch.count);
        }
        else
        {
            batch.instancingBuffer->ValidateSize(batch.count);
        }

        batch.instancingBuffer->MapBuffer(batch.matrices.data(), batch.count * CG_TYPE_SIZE_FLOAT4X4);
    }
}

void DynamicBatcher::Execute()
{
    auto pk_InstancingData = HashCache::Get()->pk_InstancingData;
    auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;

    for (auto& batch : m_batches)
    {
        if (batch.count < 1)
        {
            continue;
        }

        if (batch.count > 1)
        {
            Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
            Graphics::SetGlobalComputeBuffer(pk_InstancingData, batch.instancingBuffer->GetGraphicsID());
            Graphics::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, batch.count, batch.material.lock());
            Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
            continue;
        }

        Graphics::DrawMesh(batch.mesh.lock(), batch.submesh, batch.material.lock(), batch.matrices[0]);
    }
}

void DynamicBatcher::Execute(Ref<Material>& overrideMaterial)
{
    auto pk_InstancingData = HashCache::Get()->pk_InstancingData;
    auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;

    for (auto& batch : m_batches)
    {
        if (batch.count < 1)
        {
            continue;
        }

        if (batch.count > 1)
        {
            Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
            Graphics::SetGlobalComputeBuffer(pk_InstancingData, batch.instancingBuffer->GetGraphicsID());
            Graphics::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, batch.count, overrideMaterial);
            Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
            continue;
        }

        Graphics::DrawMesh(batch.mesh.lock(), batch.submesh, overrideMaterial, batch.matrices[0]);
    }
}
