#include "PrecompiledHeader.h"
#include "Rendering/Buffer.h"
#include <hlslmath.h>
#include <GL\glew.h>

void BufferLayout::CalculateOffsetsAndStride()
{
	size_t offset = 0;
	m_stride = 0;

	for (auto& element : m_elements)
	{
		element.Offset = offset;
		offset += element.Size;
		m_stride += element.Size;
	}
}

VertexBuffer::VertexBuffer(uint32_t size)
{
	glCreateBuffers(1, &m_graphicsId);
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

VertexBuffer::VertexBuffer(float* vertices, uint32_t size)
{
	glCreateBuffers(1, &m_graphicsId);
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(uint32_t elementCount, const BufferLayout& layout) : VertexBuffer(layout.GetStride() * elementCount)
{
	SetLayout(layout);
}

VertexBuffer::VertexBuffer(float* vertices, uint32_t elementCount, const BufferLayout& layout) : VertexBuffer(vertices, layout.GetStride() * elementCount)
{
	SetLayout(layout);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_graphicsId);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void* data, uint32_t size)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count) : m_count(count)
{
	glCreateBuffers(1, &m_graphicsId);

	// GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
	// Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
	glBindBuffer(GL_ARRAY_BUFFER, m_graphicsId);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_graphicsId);
}

void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_graphicsId);
}

void IndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
