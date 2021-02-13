#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/GraphicsObject.h"
#include <hlslmath.h>
#include <ext.hpp>

class ShaderPropertyBlock
{
    struct PropertyInfo
    {
        uint32_t type;
        uint32_t offset;
		uint32_t size;
    };

    public:
		void SetFloat(uint32_t hashId, const float* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT, values, count); }
		void SetFloat2(uint32_t hashId, const float2* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT2, values, count); }
		void SetFloat3(uint32_t hashId, const float3* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT3, values, count); }
		void SetFloat4(uint32_t hashId, const float4* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT4, values, count); }
		void SetFloat2x2(uint32_t hashId, const float2x2* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT2X2, values, count); }
		void SetFloat3x3(uint32_t hashId, const float3x3* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT3X3, values, count); }
		void SetFloat4x4(uint32_t hashId, const float4x4* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_FLOAT4X4, values, count); }
		void SetInt(uint32_t hashId, const int* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_INT, values, count); }
		void SetInt2(uint32_t hashId, const int2* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_INT2, values, count); }
		void SetInt3(uint32_t hashId, const int3* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_INT3, values, count); }
		void SetInt4(uint32_t hashId, const int4* values, uint32_t count = 1) { SetValue(hashId, CG_TYPE_INT4, values, count); }
		void SetTexture(uint32_t hashId, const GraphicsID* textureIds, uint32_t count = 1) { SetValue(hashId, CG_TYPE_TEXTURE, textureIds, count); }

		void SetFloat(uint32_t hashId, float value) { SetValue(hashId, CG_TYPE_FLOAT, &value); }
		void SetFloat2(uint32_t hashId, const float2& value) { SetValue(hashId, CG_TYPE_FLOAT2, glm::value_ptr(value)); }
		void SetFloat3(uint32_t hashId, const float3& value) { SetValue(hashId, CG_TYPE_FLOAT3,  glm::value_ptr(value)); }
		void SetFloat4(uint32_t hashId, const float4& value) { SetValue(hashId, CG_TYPE_FLOAT4, glm::value_ptr(value)); }
		void SetFloat2x2(uint32_t hashId, const float2x2& value) { SetValue(hashId, CG_TYPE_FLOAT2X2, glm::value_ptr(value)); }
		void SetFloat3x3(uint32_t hashId, const float3x3& value) { SetValue(hashId, CG_TYPE_FLOAT3X3, glm::value_ptr(value)); }
		void SetFloat4x4(uint32_t hashId, const float4x4& value) { SetValue(hashId, CG_TYPE_FLOAT4X4, glm::value_ptr(value)); }
		void SetInt(uint32_t hashId, int value) { SetValue(hashId, CG_TYPE_INT, &value); }
		void SetInt2(uint32_t hashId, const int2& value) { SetValue(hashId, CG_TYPE_INT2, glm::value_ptr(value)); }
		void SetInt3(uint32_t hashId, const int3& value) { SetValue(hashId, CG_TYPE_INT3, glm::value_ptr(value)); }
		void SetInt4(uint32_t hashId, const int4& value) { SetValue(hashId, CG_TYPE_INT4, glm::value_ptr(value)); }
		void SetTexture(uint32_t hashId, GraphicsID textureId) { SetValue(hashId, CG_TYPE_TEXTURE, &textureId); }

        void Clear();

		template<typename T>
		const T* GetElementPtr(const PropertyInfo& info) const
		{
			if (info.offset + info.size > m_data.capacity())
			{
				PK_CORE_ERROR("OOB ARRAY INDEX!");
			}

			return reinterpret_cast<const T*>(m_data.data() + info.offset);
		}

		std::unordered_map<uint32_t, PropertyInfo>::iterator begin() { return m_properties.begin(); }
		std::unordered_map<uint32_t, PropertyInfo>::iterator end() { return m_properties.end(); }
		std::unordered_map<uint32_t, PropertyInfo>::const_iterator begin() const { return m_properties.begin(); }
		std::unordered_map<uint32_t, PropertyInfo>::const_iterator end() const { return m_properties.end(); }

    private:
		template<typename T>
		void SetValue(uint32_t hashId, uint32_t type, const T* src, uint32_t count = 1)
		{
			SetValue(hashId, type, reinterpret_cast<const void*>(src), count);
		}

		void SetValue(uint32_t hashid, uint32_t type, const void* src, uint32_t count);

        uint32_t m_currentByteOffset = 0;
        std::vector<char> m_data;
        std::unordered_map<uint32_t, PropertyInfo> m_properties;
};
