#pragma once
#include "Utilities/Log.h"
#include "Rendering/Structs/BufferLayout.h"
#include <hlslmath.h>

namespace PK::Rendering::Structs
{
	using namespace PK::Math;

	class PropertyBlock
	{
		protected:
			struct PropertyInfo
			{
				PK_TYPE type = PK_TYPE::INVALID;
				ushort size = 0;
				uint offset = 0;
			};
			
		public:
	
			PropertyBlock();
			PropertyBlock(const BufferLayout& layout);
			PropertyBlock(const BufferLayout& layout, uint elementStride);
	
			virtual ~PropertyBlock() = default;
	
			inline void SetFloat(uint hashId, const float* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT, values, count); }
			inline void SetFloat2(uint hashId, const float2* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT2, values, count); }
			inline void SetFloat3(uint hashId, const float3* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT3, values, count); }
			inline void SetFloat4(uint hashId, const float4* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT4, values, count); }
			inline void SetFloat2x2(uint hashId, const float2x2* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT2X2, values, count); }
			inline void SetFloat3x3(uint hashId, const float3x3* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT3X3, values, count); }
			inline void SetFloat4x4(uint hashId, const float4x4* values, uint count = 1) { SetValue(hashId, PK_TYPE::FLOAT4X4, values, count); }
			inline void SetInt(uint hashId, const int* values, uint count = 1) { SetValue(hashId, PK_TYPE::INT, values, count); }
			inline void SetInt2(uint hashId, const int2* values, uint count = 1) { SetValue(hashId, PK_TYPE::INT2, values, count); }
			inline void SetInt3(uint hashId, const int3* values, uint count = 1) { SetValue(hashId, PK_TYPE::INT3, values, count); }
			inline void SetInt4(uint hashId, const int4* values, uint count = 1) { SetValue(hashId, PK_TYPE::INT4, values, count); }
			inline void SetUInt(uint hashId, const uint* values, uint count = 1) { SetValue(hashId, PK_TYPE::UINT, values, count); }
			inline void SetUInt2(uint hashId, const uint2* values, uint count = 1) { SetValue(hashId, PK_TYPE::UINT2, values, count); }
			inline void SetUInt3(uint hashId, const uint3* values, uint count = 1) { SetValue(hashId, PK_TYPE::UINT3, values, count); }
			inline void SetUInt4(uint hashId, const uint4* values, uint count = 1) { SetValue(hashId, PK_TYPE::UINT4, values, count); }
			inline void SetResourceHandle(uint hashId, const ulong* values, uint count = 1) { SetValue(hashId, PK_TYPE::HANDLE, values, count); }
		
			inline void SetFloat(uint hashId, float value) { SetValue(hashId, PK_TYPE::FLOAT, &value); }
			inline void SetFloat2(uint hashId, const float2& value) { SetValue(hashId, PK_TYPE::FLOAT2, glm::value_ptr(value)); }
			inline void SetFloat3(uint hashId, const float3& value) { SetValue(hashId, PK_TYPE::FLOAT3, glm::value_ptr(value)); }
			inline void SetFloat4(uint hashId, const float4& value) { SetValue(hashId, PK_TYPE::FLOAT4, glm::value_ptr(value)); }
			inline void SetFloat2x2(uint hashId, const float2x2& value) { SetValue(hashId, PK_TYPE::FLOAT2X2, glm::value_ptr(value)); }
			inline void SetFloat3x3(uint hashId, const float3x3& value) { SetValue(hashId, PK_TYPE::FLOAT3X3, glm::value_ptr(value)); }
			inline void SetFloat4x4(uint hashId, const float4x4& value) { SetValue(hashId, PK_TYPE::FLOAT4X4, glm::value_ptr(value)); }
			inline void SetInt(uint hashId, int value) { SetValue(hashId, PK_TYPE::INT, &value); }
			inline void SetInt2(uint hashId, const int2& value) { SetValue(hashId, PK_TYPE::INT2, glm::value_ptr(value)); }
			inline void SetInt3(uint hashId, const int3& value) { SetValue(hashId, PK_TYPE::INT3, glm::value_ptr(value)); }
			inline void SetInt4(uint hashId, const int4& value) { SetValue(hashId, PK_TYPE::INT4, glm::value_ptr(value)); }
			inline void SetUInt(uint hashId, uint value) { SetValue(hashId, PK_TYPE::UINT, &value); }
			inline void SetUInt2(uint hashId, const uint2& value) { SetValue(hashId, PK_TYPE::UINT2, glm::value_ptr(value)); }
			inline void SetUInt3(uint hashId, const uint3& value) { SetValue(hashId, PK_TYPE::UINT3, glm::value_ptr(value)); }
			inline void SetUInt4(uint hashId, const uint4& value) { SetValue(hashId, PK_TYPE::UINT4, glm::value_ptr(value)); }
			inline void SetResourceHandle(uint hashId, const ulong& value) { SetValue(hashId, PK_TYPE::HANDLE, &value); }
	
			void CopyFrom(PropertyBlock& from);
	
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
	
			template<typename T>
			const T* GetPropertyPtr(const uint hashId) const
			{
				return GetElementPtr<T>(m_properties.at(hashId));
			}

			template<typename T>
			const bool TryGetPropertyValue(const uint hashId, T& value) const
			{
				if (m_properties.count(hashId))
				{
					value = *GetElementPtr<T>(m_properties.at(hashId));
					return true;
				}

				return false;
			}
	
			std::unordered_map<uint, PropertyInfo>::iterator begin() { return m_properties.begin(); }
			std::unordered_map<uint, PropertyInfo>::iterator end() { return m_properties.end(); }
			std::unordered_map<uint, PropertyInfo>::const_iterator begin() const { return m_properties.begin(); }
			std::unordered_map<uint, PropertyInfo>::const_iterator end() const { return m_properties.end(); }
		
		protected:
			template<typename T>
			void SetValue(uint hashId, PK_TYPE type, const T* src, uint count = 1)
			{
				SetValue(hashId, type, reinterpret_cast<const void*>(src), count);
			}
		
			void SetValue(uint hashid, PK_TYPE type, const void* src, uint count);
	
			bool m_explicitLayout = false;
			uint m_currentByteOffset = 0;
			std::vector<char> m_data;
			std::unordered_map<uint, PropertyInfo> m_properties;
	};
}