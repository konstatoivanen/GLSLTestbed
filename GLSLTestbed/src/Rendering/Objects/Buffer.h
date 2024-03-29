#pragma once
#include "Core/BufferView.h"
#include "Rendering/Objects/GraphicsObject.h"
#include "Rendering/Structs/BufferLayout.h"
#include "Rendering/Structs/PropertyBlock.h"
#include <glad/glad.h>
#include <hlslmath.h>

namespace PK::Rendering::Objects
{
	using namespace Structs;

	class VertexBuffer : public GraphicsObject
	{
		public:
			VertexBuffer(size_t size);
			VertexBuffer(const void* vertices, size_t size, bool immutable);
			VertexBuffer(size_t elementCount, const BufferLayout& layout);
			VertexBuffer(const void* vertices, size_t elementCount, const BufferLayout& layout, bool immutable);
			~VertexBuffer();
	
			void SetData(const void* data, size_t size);
		
			inline const BufferLayout& GetLayout() const { return m_layout; }
			inline void SetLayout(const BufferLayout& layout) { m_layout = layout; }
	
		private:
			BufferLayout m_layout;
			bool m_immutable;
	};
	
	class IndexBuffer : public GraphicsObject
	{
		public:
			IndexBuffer(uint* indices, uint count, bool immutable);
			~IndexBuffer();
		
			inline uint GetCount() const { return m_count; }

		private:
			uint m_count;
			bool m_immutable;
	};
	
	class ConstantBuffer : public GraphicsObject, public PropertyBlock
	{
		public:
			ConstantBuffer(const BufferLayout& layout);
			~ConstantBuffer();
			void FlushBuffer();
		private:
	};
	
	class ComputeBuffer : public GraphicsObject
	{
		public:
			ComputeBuffer(const BufferLayout& layout, uint count, bool immutable, uint flags);
			~ComputeBuffer();
			void Resize(uint newCount);
			void ValidateSize(uint newCount);
			void MapBuffer(const void* data, size_t offset, size_t size);
			void MapBuffer(const void* data, size_t size);
			void SubmitData(const void* data, size_t offset, size_t size);
			void Clear(uint32_t clearValue = 0u) const;

			void* BeginMapBuffer();
			void* BeginMapBufferRange(size_t offset, size_t size);
	
			template<typename T>
			Core::BufferView<T> BeginMapBuffer()
			{
				return { reinterpret_cast<T*>(BeginMapBuffer()), GetSize() / sizeof(T) };
			}

			template<typename T>
			Core::BufferView<T> BeginMapBufferRange(size_t offset, size_t count)
			{
				auto tsize = sizeof(T);
				auto mapSize = tsize * count + tsize * offset;
				auto bufSize = GetSize();

				PK_CORE_ASSERT(mapSize <= bufSize, "Map buffer range exceeds buffer bounds, map size: %i, buffer size: %i", mapSize, bufSize);

				return { reinterpret_cast<T*>(BeginMapBufferRange(offset * tsize, count * tsize)), count };
			}
	
			void EndMapBuffer();
			
			inline const BufferLayout& GetLayout() const { return m_layout; }
			inline size_t GetSize() const { return m_count * m_layout.GetPaddedStride(); }
			inline size_t GetStride() const { return m_layout.GetPaddedStride(); }
			inline size_t GetCount() const { return m_count; }
		private:
			BufferLayout m_layout;
			size_t m_count;
			GLenum m_usage;
			bool m_immutable;
	};
}