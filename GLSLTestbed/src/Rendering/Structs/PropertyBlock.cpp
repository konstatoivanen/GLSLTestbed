#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"
#include "Rendering/Structs/PropertyBlock.h"

PropertyBlock::PropertyBlock() : m_explicitLayout(false), m_currentByteOffset(0)
{
}

PropertyBlock::PropertyBlock(const BufferLayout& layout, uint elementStride) : m_explicitLayout(true), m_currentByteOffset(0)
{
	m_data.resize(layout.GetStride());

	for (const auto& element : layout)
	{
		PK_CORE_ASSERT(CGType::Size(element.Type) % elementStride == 0, "Property block doesnt support elements that are not of ")
		m_properties[StringHashID::StringToID(element.Name)] = { element.Type, element.Size, m_currentByteOffset };
		m_currentByteOffset += element.Size;
	}
}

PropertyBlock::PropertyBlock(const BufferLayout& layout) : PropertyBlock(layout, 16)
{
}

void PropertyBlock::SetValue(uint hashid, ushort type, const void* src, uint count)
{
	if (type == CG_TYPE_ERROR || count == 0)
	{
		PK_CORE_ERROR("INVALID DATA TYPE OR COUNT!");
	}

	auto size = (ushort)(CGType::Size(type) * count);
	auto& info = m_properties[hashid];

	if (info.size == 0)
	{
		PK_CORE_ASSERT(!m_explicitLayout, "Cannot add elements to explicitly mapped property block!");

		if (m_currentByteOffset >= m_data.capacity())
		{
			m_data.reserve(m_data.capacity() + size);
		}

		info = { type, size, m_currentByteOffset };
		m_currentByteOffset += size;
	}
	else if (info.size > size || info.type != type)
	{
		PK_CORE_ERROR("INVALID DATA FORMAT!");
	}

	memcpy(m_data.data() + info.offset, src, size);
}

void PropertyBlock::CopyValues(const PropertyBlock& from)
{
	auto& propm = m_properties;
	auto& propt = from.m_properties;

	for (auto& prop : propm)
	{
		if (propt.count(prop.first) < 1)
		{
			continue;
		}

		auto& theirs = propt.at(prop.first);
		auto& mine = prop.second;

		// missmatch
		if (mine.size != theirs.size || mine.type != theirs.type)
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