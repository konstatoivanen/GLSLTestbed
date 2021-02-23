#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Structs/BufferLayout.h"
#include <hlslmath.h>
#include <ext.hpp>

class PropertyBlock
{
	protected:
		struct PropertyInfo
		{
			ushort type;
			ushort size;
			uint offset;
		};
		
	public:
		PropertyBlock();
		PropertyBlock(const BufferLayout& layout);
		PropertyBlock(const BufferLayout& layout, uint elementStride);

		void SetFloat(uint hashId, const float* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT, values, count); }
		void SetFloat2(uint hashId, const float2* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT2, values, count); }
		void SetFloat3(uint hashId, const float3* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT3, values, count); }
		void SetFloat4(uint hashId, const float4* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT4, values, count); }
		void SetFloat2x2(uint hashId, const float2x2* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT2X2, values, count); }
		void SetFloat3x3(uint hashId, const float3x3* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT3X3, values, count); }
		void SetFloat4x4(uint hashId, const float4x4* values, uint count = 1) { SetValue(hashId, CG_TYPE_FLOAT4X4, values, count); }
		void SetInt(uint hashId, const int* values, uint count = 1) { SetValue(hashId, CG_TYPE_INT, values, count); }
		void SetInt2(uint hashId, const int2* values, uint count = 1) { SetValue(hashId, CG_TYPE_INT2, values, count); }
		void SetInt3(uint hashId, const int3* values, uint count = 1) { SetValue(hashId, CG_TYPE_INT3, values, count); }
		void SetInt4(uint hashId, const int4* values, uint count = 1) { SetValue(hashId, CG_TYPE_INT4, values, count); }
	
		void SetFloat(uint hashId, float value) { SetValue(hashId, CG_TYPE_FLOAT, &value); }
		void SetFloat2(uint hashId, const float2& value) { SetValue(hashId, CG_TYPE_FLOAT2, glm::value_ptr(value)); }
		void SetFloat3(uint hashId, const float3& value) { SetValue(hashId, CG_TYPE_FLOAT3, glm::value_ptr(value)); }
		void SetFloat4(uint hashId, const float4& value) { SetValue(hashId, CG_TYPE_FLOAT4, glm::value_ptr(value)); }
		void SetFloat2x2(uint hashId, const float2x2& value) { SetValue(hashId, CG_TYPE_FLOAT2X2, glm::value_ptr(value)); }
		void SetFloat3x3(uint hashId, const float3x3& value) { SetValue(hashId, CG_TYPE_FLOAT3X3, glm::value_ptr(value)); }
		void SetFloat4x4(uint hashId, const float4x4& value) { SetValue(hashId, CG_TYPE_FLOAT4X4, glm::value_ptr(value)); }
		void SetInt(uint hashId, int value) { SetValue(hashId, CG_TYPE_INT, &value); }
		void SetInt2(uint hashId, const int2& value) { SetValue(hashId, CG_TYPE_INT2, glm::value_ptr(value)); }
		void SetInt3(uint hashId, const int3& value) { SetValue(hashId, CG_TYPE_INT3, glm::value_ptr(value)); }
		void SetInt4(uint hashId, const int4& value) { SetValue(hashId, CG_TYPE_INT4, glm::value_ptr(value)); }
	
		void CopyValues(const PropertyBlock& from);

		virtual void Clear();
	
		template<typename T>
		const T* GetElementPtr(const PropertyInfo& info) const
		{
			if (info.offset + (size_t)info.size > m_data.capacity())
			{
				PK_CORE_ERROR("OOB ARRAY INDEX! idx: %i, capacity: %i", info.offset + info.size, m_data.capacity());
			}
	
			return reinterpret_cast<const T*>(m_data.data() + info.offset);
		}

		std::unordered_map<uint, PropertyInfo>::iterator begin() { return m_properties.begin(); }
		std::unordered_map<uint, PropertyInfo>::iterator end() { return m_properties.end(); }
		std::unordered_map<uint, PropertyInfo>::const_iterator begin() const { return m_properties.begin(); }
		std::unordered_map<uint, PropertyInfo>::const_iterator end() const { return m_properties.end(); }
	
	protected:
		template<typename T>
		void SetValue(uint hashId, ushort type, const T* src, uint count = 1)
		{
			SetValue(hashId, type, reinterpret_cast<const void*>(src), count);
		}
	
		void SetValue(uint hashid, ushort type, const void* src, uint count);

		bool m_explicitLayout;
		uint m_currentByteOffset = 0;
		std::vector<char> m_data;
		std::unordered_map<uint, PropertyInfo> m_properties;
};
