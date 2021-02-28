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

VertexBuffer::VertexBuffer(float* vertices, size_t size)
{
	glCreateBuffers(1, &m_graphicsId);
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(size_t elementCount, const BufferLayout& layout) : VertexBuffer(layout.GetStride() * elementCount)
{
	SetLayout(layout);
}

VertexBuffer::VertexBuffer(float* vertices, size_t elementCount, const BufferLayout& layout) : VertexBuffer(vertices, layout.GetStride() * elementCount)
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
	glBufferData(GL_UNIFORM_BUFFER, layout.GetStride(), nullptr, GL_DYNAMIC_DRAW);
}

ConstantBuffer::~ConstantBuffer()
{
	glDeleteBuffers(1, &m_graphicsId);
}

void ConstantBuffer::FlushBufer()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_graphicsId);
	auto dest = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(dest, m_data.data(), m_data.size());
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

ComputeBuffer::ComputeBuffer(const BufferLayout& layout, uint count) : m_layout(layout)
{
	glGenBuffers(1, &m_graphicsId);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (size_t)layout.GetStride() * (size_t)count, nullptr, GL_DYNAMIC_DRAW);
}

ComputeBuffer::~ComputeBuffer()
{
	glDeleteBuffers(1, &m_graphicsId);
}

void ComputeBuffer::SetData(const void* data, size_t offset, size_t size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
	auto dest = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(reinterpret_cast<char*>(dest) + offset, data, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ComputeBuffer::SetData(const void* data, size_t size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_graphicsId);
	auto dest = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(dest, data, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
