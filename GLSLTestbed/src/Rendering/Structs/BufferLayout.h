#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"
#include <hlslmath.h>

namespace PK::Rendering::Structs
{
	using namespace PK::Math;
	using namespace PK::Utilities;

	struct BufferElement
	{
		uint32_t NameHashId = 0;
		CG_TYPE Type = CG_TYPE::INVALID;
		ushort Size = 0;
		size_t Offset = 0;
		bool Normalized = false;
	
		BufferElement() = default;
	
		BufferElement(CG_TYPE type, const std::string& name, ushort count = 1, bool normalized = false) : NameHashId(StringHashID::StringToID(name)), Type(type), Size(Convert::Size(type)* count), Offset(0), Normalized(normalized)
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
		
			inline uint GetStride() const { return m_stride; }
			inline uint GetPaddedStride() const { return m_paddedStride; }
			inline const std::vector<BufferElement>& GetElements() const { return m_elements; }
		
			std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
			std::vector<BufferElement>::iterator end() { return m_elements.end(); }
			std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
			std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }
	
		private:
			void CalculateOffsetsAndStride();
		
		private:
			std::vector<BufferElement> m_elements;
			uint m_stride = 0;
			uint m_paddedStride = 0;
	};
}