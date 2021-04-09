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
		
			const BufferLayout& GetLayout() const { return m_layout; }
			void SetLayout(const BufferLayout& layout) { m_layout = layout; }
	
		private:
			BufferLayout m_layout;
			bool m_immutable;
	};
	
	class IndexBuffer : public GraphicsObject
	{
		public:
			IndexBuffer(uint* indices, uint count, bool immutable);
			~IndexBuffer();
		
			uint GetCount() const { return m_count; }

		private:
			uint m_count;
			bool m_immutable;
	};
	
	class ConstantBuffer : public GraphicsObject, public PropertyBlock
	{
		public:
			ConstantBuffer(const BufferLayout& layout);
			~ConstantBuffer();
			void FlushBufer();
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
				auto tcount = (uint)(m_count / (tsize / (float)GetStride()));
				PK_CORE_ASSERT(count <= tcount, "Map buffer range exceeds buffer bounds");

				return { reinterpret_cast<T*>(BeginMapBufferRange(offset * tsize, count * tsize)), count };
			}
	
			void EndMapBuffer();
			
			const BufferLayout& GetLayout() const { return m_layout; }
			size_t GetSize() const { return m_count * m_layout.GetPaddedStride(); }
			size_t GetStride() const { return m_layout.GetPaddedStride(); }
			size_t GetCount() const { return m_count; }
		private:
			BufferLayout m_layout;
			size_t m_count;
			GLenum m_usage;
			bool m_immutable;
	};
}