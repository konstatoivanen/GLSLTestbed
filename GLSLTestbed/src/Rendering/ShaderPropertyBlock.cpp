#include "PrecompiledHeader.h"
#include "Rendering/ShaderPropertyBlock.h"

void ShaderPropertyBlock::Clear()
{
	m_currentByteOffset = 0;
	m_properties.clear();
}

void ShaderPropertyBlock::SetValue(uint32_t hashid, uint32_t type, const void* src, uint32_t count)
{
	if (type == CG_TYPE_ERROR || count == 0)
	{
		PK_CORE_ASSERT(false, "INVALID DATA TYPE OR COUNT!");
	}

	auto size = CGType::Size(type) * count;
	auto& info = m_properties[hashid];

	if (info.size == 0)
	{
		if (m_currentByteOffset >= m_data.capacity())
		{
			m_data.reserve(m_data.capacity() + size);
		}

		info = { type, m_currentByteOffset, size };
		m_currentByteOffset += size;
	}
	else if (info.size > size || info.type != type)
	{
		PK_CORE_ERROR("INVALID DATA FORMAT!");
	}

	memcpy(m_data.data() + info.offset, src, size);
}