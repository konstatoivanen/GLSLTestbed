#include "PrecompiledHeader.h"
#include "Rendering/Objects/Buffer.h"

namespace PK::Rendering::Structs
{
	void BufferLayout::CalculateOffsetsAndStride()
	{
		auto maxSize = 0;
		m_stride = 0;
	
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
	
	VertexBuffer::VertexBuffer(const void* vertices, size_t size)
	{
		glCreateBuffers(1, &m_graphicsId);
		glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
	
	VertexBuffer::VertexBuffer(size_t elementCount, const BufferLayout& layout) : VertexBuffer(layout.GetStride() * elementCount)
	{
		SetLayout(layout);
	}
	
	VertexBuffer::VertexBuffer(const void* vertices, size_t elementCount, const BufferLayout& layout) : VertexBuffer(vertices, layout.GetStride() * elementCount)
	{
		SetLayout(layout);
	}
	
	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	void VertexBuffer::SetData(const void* data, size_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	
	IndexBuffer::IndexBuffer(uint* indices, uint count) : m_count(count)
	{
		glCreateBuffers(1, &m_graphicsId);
		glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint), indices, GL_STATIC_DRAW);
	}
	
	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	ConstantBuffer::ConstantBuffer(const BufferLayout& layout) : PropertyBlock(layout, 16)
	{
		glGenBuffers(1, &m_graphicsId);
		glBindBuffer(GL_UNIFORM_BUFFER, m_graphicsId);
		glBufferData(GL_UNIFORM_BUFFER, layout.GetStride(), nullptr, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
	ConstantBuffer::~ConstantBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	void ConstantBuffer::FlushBufer()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_graphicsId);
		glInvalidateBufferData(GL_UNIFORM_BUFFER);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, m_data.size(), m_data.data());
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	
	ComputeBuffer::ComputeBuffer(const BufferLayout& layout, uint count, GLenum usage) : m_usage(usage), m_count(count), m_layout(layout)
	{
		auto stride = m_layout.GetPaddedStride();
		auto size = GetSize();
		glGenBuffers(1, &m_graphicsId);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, usage);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	
	ComputeBuffer::~ComputeBuffer()
	{
		glDeleteBuffers(1, &m_graphicsId);
	}
	
	void ComputeBuffer::Resize(uint newCount)
	{
		if (newCount == m_count)
		{
			return;
		}
	
		m_count = newCount;
		auto size = GetSize();
		
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
		glInvalidateBufferData(GL_SHADER_STORAGE_BUFFER);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, m_usage);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
		glInvalidateBufferData(GL_SHADER_STORAGE_BUFFER);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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