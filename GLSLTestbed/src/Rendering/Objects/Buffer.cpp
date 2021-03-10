#include "PrecompiledHeader.h"
#include "Rendering/Objects/Buffer.h"

void BufferLayout::CalculateOffsetsAndStride()
{
	m_stride = 0;

	for (auto& element : m_elements)
	{
		element.Offset = m_stride;
		m_stride += element.Size;
	}
}

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
}

ComputeBuffer::ComputeBuffer(const BufferLayout& layout, uint count) : m_count(count), m_layout(layout)
{
	auto stride = m_layout.GetStride();
	auto size = GetSize();
	glGenBuffers(1, &m_graphicsId);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

ComputeBuffer::~ComputeBuffer()
{
	glDeleteBuffers(1, &m_graphicsId);
}

void ComputeBuffer::SetData(const void* data, size_t offset, size_t size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
	auto dest = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(dest, data, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ComputeBuffer::SetData(const void* data, size_t size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
	auto dest = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY | GL_MAP_INVALIDATE_BUFFER_BIT);
	memcpy(dest, data, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
