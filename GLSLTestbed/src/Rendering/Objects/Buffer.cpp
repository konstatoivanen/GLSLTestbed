#include "PrecompiledHeader.h"
#include "Rendering/Objects/Buffer.h"
#include <hlslmath.h>
#include <glad/glad.h>

void BufferLayout::CalculateOffsetsAndStride()
{
	m_stride = 0;

	for (auto& element : m_elements)
	{
		element.Offset = m_stride;
		m_stride += element.Size;
	}
}

VertexBuffer::VertexBuffer(uint size)
{
	glCreateBuffers(1, &m_graphicsId);
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(float* vertices, uint size)
{
	glCreateBuffers(1, &m_graphicsId);
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(uint elementCount, const BufferLayout& layout) : VertexBuffer(layout.GetStride() * elementCount)
{
	SetLayout(layout);
}

VertexBuffer::VertexBuffer(float* vertices, uint elementCount, const BufferLayout& layout) : VertexBuffer(vertices, layout.GetStride() * elementCount)
{
	SetLayout(layout);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_graphicsId);
}

void VertexBuffer::SetData(const void* data, uint size)
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
