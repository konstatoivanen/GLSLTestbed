#include "PrecompiledHeader.h"
#include "BufferPool.h"
#include "Utilities/Utilities.h"

namespace PK::Rendering
{
    Ref<ComputeBuffer>& BufferPool::GetBuffer(size_t size)
    {
        auto minStride = CG_TYPE_SIZE_FLOAT4;
        auto count = size / minStride;
    
        Utilities::ValidateVectorSize(m_buffers, (size_t)m_usedCount + 1);
    
        if (m_buffers.at(m_usedCount) == nullptr)
        {
            m_buffers[m_usedCount] = CreateRef<ComputeBuffer>(BufferLayout({ {CG_TYPE::FLOAT4, "DATA"} }), (uint)count);
        }
        else
        {
            m_buffers[m_usedCount]->ValidateSize((uint)count);
        }
    
        return m_buffers[m_usedCount++];
    }
    
    void BufferPool::Reset()
    {
        m_usedCount = 0;
    }
}