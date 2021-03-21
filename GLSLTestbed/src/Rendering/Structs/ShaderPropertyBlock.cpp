#include "PrecompiledHeader.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"

namespace PK::Rendering::Structs
{
	void ShaderPropertyBlock::SetKeyword(uint32_t hashId, bool value)
	{
		auto iterator = std::find(m_keywords.begin(), m_keywords.end(), hashId);
	
		if (iterator == m_keywords.end())
		{
			if (value)
			{
				m_keywords.push_back(hashId);
			}
	
			return;
		}
	
		if (!value)
		{
			m_keywords.erase(iterator);
		}
	}

    void ShaderPropertyBlock::SetKeywords(std::initializer_list<uint32_t> hashIds)
    {
		m_keywords.resize(hashIds.size());
		memcpy(m_keywords.data(), hashIds.begin(), sizeof(uint32_t) * hashIds.size());
    }
	
	void ShaderPropertyBlock::Clear()
	{
		PropertyBlock::Clear();
		m_keywords.clear();
	}
}