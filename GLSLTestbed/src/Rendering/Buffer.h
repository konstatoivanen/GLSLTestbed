#pragma once
#include "PrecompiledHeader.h"
#include "Rendering/GraphicsObject.h"
#include <GL/glew.h>
#include <hlslmath.h>

struct BufferElement
{
	std::string Name;
	uint32_t Type;
	uint32_t Size;
	size_t Offset;
	bool Normalized;

	BufferElement() = default;

	BufferElement(uint32_t type, const std::string& name, bool normalized = false) : Name(name), Type(type), Size(CGType::Size(type)), Offset(0), Normalized(normalized)
	{
	}
};

class BufferLayout
{
	public:
		BufferLayout() {}
	
		BufferLayout(std::initializer_list<BufferElement> elements) : m_elements(elements)
		{
			CalculateOffsetsAndStride();
		}
	
		uint32_t GetStride() const { return m_stride; }
		const std::vector<BufferElement>& GetElements() const { return m_elements; }
	
		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

	private:
		void CalculateOffsetsAndStride();

	private:
		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;
};

class VertexBuffer : public GraphicsObject
{
	public:
		VertexBuffer(uint32_t size);
		VertexBuffer(float* vertices, uint32_t size);
		VertexBuffer(uint32_t elementCount, const BufferLayout& layout);
		VertexBuffer(float* vertices, uint32_t elementCount, const BufferLayout& layout);
		~VertexBuffer();
	
		void Bind() const;
		void Unbind() const;
	
		void SetData(const void* data, uint32_t size);
	
		const BufferLayout& GetLayout() const { return m_layout; }
		void SetLayout(const BufferLayout& layout) { m_layout = layout; }

	private:
		BufferLayout m_layout;
};

class IndexBuffer : public GraphicsObject
{
	public:
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();
	
		void Bind() const;
		void Unbind() const;
	
		uint32_t GetCount() const { return m_count; }

	private:
		uint32_t m_count;
};