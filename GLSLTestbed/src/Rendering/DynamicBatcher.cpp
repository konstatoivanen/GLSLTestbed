#include "PrecompiledHeader.h"
#include "Utilities/HashCache.h"
#include "Rendering/DynamicBatcher.h"
#include "Rendering/Graphics.h"

void DynamicBatcher::ResetCapacities()
{
    for (auto& batch : m_batches)
    {
        batch.count = 0;
        batch.activeIndex = 0;
    }
}

void DynamicBatcher::BuildCapacity(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material)
{
    auto materialId = material.lock()->GetAssetID();
    auto meshId = mesh.lock()->GetGraphicsID();

    auto submeshKey = ((uint)submesh << 24) | ((uint)materialId & 0xFFFFFF);
    auto batchKey = (ulong)meshId << 32 | ((ulong)(uint)submeshKey & 0xFFFFFFFF);

    DynamicBatch* batch = nullptr;

    if (m_batchmap.count(batchKey))
    {
        m_batches.at(m_batchmap.at(batchKey)).count++;
    }
    else
    {
        auto index = (uint)m_batches.size();
        m_batchmap[batchKey] = index;
        m_batches.push_back(DynamicBatch());
        batch = &m_batches.at(index);
        batch->material = material;
        batch->mesh = mesh;
        batch->count = 1;
    }
}

void DynamicBatcher::BeginMapBuffers()
{
    uint totalCount = 0;

    for (auto& batch : m_batches)
    {
        batch.offset = totalCount;
        totalCount += batch.count;
    }

    if (m_instancingBuffer == nullptr)
    {
        m_instancingBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE_FLOAT4X4, "Matrix" } }), totalCount);
    }
    else
    {
        m_instancingBuffer->ValidateSize(totalCount);
    }

    m_mappedBuffer = m_instancingBuffer->BeginMapBuffer<float4x4>();
}

void DynamicBatcher::EndMapBuffers()
{
    m_instancingBuffer->EndMapBuffer();
    m_mappedBuffer.data = nullptr;
}

void DynamicBatcher::QueueDraw(Weak<Mesh>& mesh, uint submesh, Weak<Material>& material, const float4x4& matrix)
{
    auto materialId = material.lock()->GetAssetID();
    auto meshId = mesh.lock()->GetGraphicsID();
    auto submeshKey = ((uint)submesh << 24) | ((uint)materialId & 0xFFFFFF);
    auto batchKey = (ulong)meshId << 32 | ((ulong)(uint)submeshKey & 0xFFFFFFFF);
    auto batch = &m_batches.at(m_batchmap.at(batchKey));
    m_mappedBuffer[batch->offset + batch->activeIndex] = matrix;
    ++batch->activeIndex;
}

void DynamicBatcher::Execute()
{
    auto pk_InstancingData = HashCache::Get()->pk_InstancingData;
    auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
    Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
    Graphics::SetGlobalComputeBuffer(pk_InstancingData, m_instancingBuffer->GetGraphicsID());

    for (auto& batch : m_batches)
    {
        if (batch.count < 1)
        {
            continue;
        }

        Graphics::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, batch.offset, batch.count, batch.material.lock());
    }

    Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
}

void DynamicBatcher::Execute(Ref<Material>& overrideMaterial)
{
    auto pk_InstancingData = HashCache::Get()->pk_InstancingData;
    auto PK_ENABLE_INSTANCING = HashCache::Get()->PK_ENABLE_INSTANCING;
    Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, true);
    Graphics::SetGlobalComputeBuffer(pk_InstancingData, m_instancingBuffer->GetGraphicsID());

    for (auto& batch : m_batches)
    {
        if (batch.count < 1)
        {
            continue;
        }

        Graphics::DrawMeshInstanced(batch.mesh.lock(), batch.submesh, batch.offset, batch.count, overrideMaterial);
    }

    Graphics::SetGlobalKeyword(PK_ENABLE_INSTANCING, false);
}
