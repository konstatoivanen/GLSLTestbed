#pragma once
#include "PrecompiledHeader.h"
#include <hlslmath.h>

struct BufferElement
{
	std::string Name;
	ushort Type;
	ushort Size;
	size_t Offset;
	bool Normalized;

	BufferElement() = default;

	BufferElement(ushort type, const std::string& name, ushort count = 1, bool normalized = false) : Name(name), Type(type), Size(CGConvert::Size(type)* count), Offset(0), Normalized(normalized)
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

		BufferLayout(std::vector<BufferElement> elements) : m_elements(elements)
		{
			CalculateOffsetsAndStride();
		}
	
		uint GetStride() const { return m_stride; }
		const std::vector<BufferElement>& GetElements() const { return m_elements; }
	
		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

	private:
		void CalculateOffsetsAndStride();
	
	private:
		std::vector<BufferElement> m_elements;
		uint m_stride = 0;
};