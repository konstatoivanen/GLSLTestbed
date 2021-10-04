#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"
#include "Rendering/Structs/PropertyBlock.h"

namespace PK::Rendering::Structs
{
	using namespace PK::Utilities;
	using namespace PK::Math;

	PropertyBlock::PropertyBlock() : m_explicitLayout(false), m_currentByteOffset(0)
	{
	}
	
	PropertyBlock::PropertyBlock(const BufferLayout& layout, uint elementStride) : m_explicitLayout(true), m_currentByteOffset(0)
	{
		m_data.resize(layout.GetStride());
	
		for (const auto& element : layout)
		{
			//auto elementSize = Convert::Size(element.Type);
			//ushort paddedSize = elementStride * (ushort)ceil((float)elementSize / elementStride);
			//ushort paddedSizeFull = paddedSize * (element.Size / elementSize);
			m_properties[element.NameHashId] = { element.Type, element.Size, m_currentByteOffset };
			m_currentByteOffset += element.Size;
		}
	}
	
	PropertyBlock::PropertyBlock(const BufferLayout& layout) : PropertyBlock(layout, 16)
	{
	}
	
	void PropertyBlock::SetValue(uint hashid, PK_TYPE type, const void* src, uint count)
	{
		if (type == PK_TYPE::INVALID || count == 0)
		{
			PK_CORE_ERROR("INVALID DATA TYPE OR COUNT!");
		}
	
		auto size = (ushort)(Convert::Size(type) * count);
		auto& info = m_properties[hashid];
	
		if (info.size == 0)
		{
			PK_CORE_ASSERT(!m_explicitLayout, "Cannot add elements to explicitly mapped property block!");
	
			if (m_currentByteOffset >= m_data.size())
			{
				m_data.resize(m_data.size() + size);
			}
	
			info = { type, size, m_currentByteOffset };
			m_currentByteOffset += size;
		}
		else if (info.size < size || info.type != type)
		{
			PK_CORE_ERROR("INVALID DATA FORMAT! %s", StringHashID::IDToString(hashid).c_str());
		}
	
		memcpy(m_data.data() + info.offset, src, size);
	}
	
	void PropertyBlock::CopyFrom(PropertyBlock& from)
	{
		auto& propsm = m_properties;
		auto& propst = from.m_properties;
	
		for (auto& prop : propsm)
		{
			if (propst.count(prop.first) < 1)
			{
				continue;
			}
	
			auto& theirs = propst.at(prop.first);
			auto& mine = prop.second;
	
			// missmatch
			if (mine.size < theirs.size || mine.type != theirs.type)
			{
				continue;
			}
	
			auto src = from.m_data.data() + theirs.offset;
			auto dst = m_data.data() + mine.offset;
	
			memcpy(dst, src, mine.size);
		}
	}
	
	void PropertyBlock::Clear()
	{
		m_currentByteOffset = 0;
		m_properties.clear();
	}
}