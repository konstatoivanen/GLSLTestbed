#include "PrecompiledHeader.h"
#include "Rendering/Objects/Buffer.h"

namespace PK::Rendering::Structs
{
	void BufferLayout::CalculateOffsetsAndStride()
	{
		auto maxSize = 0;
		m_stride = 0;
	
		// @TODO For std140/430 padding is calculated for variables in a way that their offset is a multiple of their size.
		// For example if 16b variable is preceeded by an 8b variable there will be 8b of padding between them. Fix this padding calculation to reflect that.
		for (auto& element : m_elements)
		{
			element.Offset = m_stride;
			m_stride += element.Size;

			if (element.Size > maxSize)
			{
				maxSize = element.Size;
			}
		}

		m_paddedStride = maxSize * (uint)glm::ceil(m_stride / (float)maxSize);
	}
}

namespace PK::Rendering::Objects
{
	using namespace Structs;
	
	VertexBuffer::VertexBuffer(size_t size)
	{
		glCreateBuffers(1, &m_graphicsId);
		glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}
	
	VertexBuffer::VertexBuffer(const void* vertices, size_t size, bool immutable) : m_immutable(immutable)
	{
		glCreateBuffers(1, &m_graphicsId);
		glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);

		if (immutable)
		{
			glBufferStorage(GL_ARRAY_BUFFER, size, vertices, GL_NONE);
		}
		else
		{
			glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
		}
	}
	
	VertexBuffer::VertexBuffer(size_t elementCount, const BufferLayout& layout) : VertexBuffer(layout.GetStride() * elementCount)
	{
		SetLayout(layout);
	}
	
	VertexBuffer::VertexBuffer(const void* vertices, size_t elementCount, const BufferLayout& layout, bool immutable) : VertexBuffer(vertices, layout.GetStride() * elementCount, immutable)
	{
		SetLayout(layout);
	}
	
	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	void VertexBuffer::SetData(const void* data, size_t size)
	{
		PK_CORE_ASSERT(!m_immutable, "Attempting to modify an immutable vertex buffer");
		glNamedBufferSubData(m_graphicsId, 0, size, data);
	}
	
	IndexBuffer::IndexBuffer(uint* indices, uint count, bool immutable) : m_count(count), m_immutable(immutable)
	{
		glCreateBuffers(1, &m_graphicsId);
		glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);

		if (immutable)
		{
			glBufferStorage(GL_ARRAY_BUFFER, count * sizeof(uint), indices, GL_NONE);
		}
		else
		{
			glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
		}
	}
	
	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	ConstantBuffer::ConstantBuffer(const BufferLayout& layout) : PropertyBlock(layout, 16)
	{
		glGenBuffers(1, &m_graphicsId);
		glBindBuffer(GL_UNIFORM_BUFFER, m_graphicsId);
		glBufferStorage(GL_UNIFORM_BUFFER, layout.GetStride(), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
	ConstantBuffer::~ConstantBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	void ConstantBuffer::FlushBuffer()
	{
		glInvalidateBufferSubData(m_graphicsId, 0, m_data.size());
		glNamedBufferSubData(m_graphicsId, 0, m_data.size(), m_data.data());
	}
	
	ComputeBuffer::ComputeBuffer(const BufferLayout& layout, uint count, bool immutable, uint flags) : m_usage(flags), m_count(count), m_layout(layout), m_immutable(immutable)
	{
		auto stride = m_layout.GetPaddedStride();
		auto size = GetSize();
		glGenBuffers(1, &m_graphicsId);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);

		if (m_immutable)
		{
			glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, nullptr, flags);
		}
		else
		{
			glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, flags);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	
	ComputeBuffer::~ComputeBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	void ComputeBuffer::Resize(uint newCount)
	{
		PK_CORE_ASSERT(!m_immutable, "Cannot resize an immutable buffer!");

		if (newCount == m_count)
		{
			return;
		}
	
		m_count = newCount;
		auto size = GetSize();
		
		glInvalidateBufferData(m_graphicsId);
		glNamedBufferData(m_graphicsId, size, nullptr, m_usage);
	}
	
	void ComputeBuffer::ValidateSize(uint newCount)
	{
		if (newCount > m_count)
		{
			Resize(newCount);
		}
	}
	
	void ComputeBuffer::MapBuffer(const void* data, size_t offset, size_t size)
	{
		auto dest = glMapNamedBufferRange(m_graphicsId, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(dest, data, size);
		glUnmapNamedBuffer(m_graphicsId);
	}
	
	void ComputeBuffer::MapBuffer(const void* data, size_t size)
	{
		auto dest = glMapNamedBuffer(m_graphicsId, GL_WRITE_ONLY | GL_MAP_INVALIDATE_BUFFER_BIT);
		memcpy(dest, data, size);
		glUnmapNamedBuffer(m_graphicsId);
	}
	
	void ComputeBuffer::SubmitData(const void* data, size_t offset, size_t size)
	{
		glInvalidateBufferSubData(m_graphicsId, offset, size);
		glNamedBufferSubData(m_graphicsId, offset, size, data);
	}

	void ComputeBuffer::Clear(uint32_t clearValue) const
	{
		glClearNamedBufferData(m_graphicsId, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &clearValue);
	}

    void* ComputeBuffer::BeginMapBufferRange(size_t offset, size_t size)
    {
		return glMapNamedBufferRange(m_graphicsId, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    }
	
	void* ComputeBuffer::BeginMapBuffer()
	{
		return glMapNamedBuffer(m_graphicsId, GL_WRITE_ONLY | GL_MAP_INVALIDATE_BUFFER_BIT);
	}
	
	void ComputeBuffer::EndMapBuffer()
	{
		glUnmapNamedBuffer(m_graphicsId);
	}
}