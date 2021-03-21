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
			VertexBuffer(const void* vertices, size_t size);
			VertexBuffer(size_t elementCount, const BufferLayout& layout);
			VertexBuffer(const void* vertices, size_t elementCount, const BufferLayout& layout);
			~VertexBuffer();
	
			void SetData(const void* data, size_t size);
		
			const BufferLayout& GetLayout() const { return m_layout; }
			void SetLayout(const BufferLayout& layout) { m_layout = layout; }
	
		private:
			BufferLayout m_layout;
	};
	
	class IndexBuffer : public GraphicsObject
	{
		public:
			IndexBuffer(uint* indices, uint count);
			~IndexBuffer();
		
			uint GetCount() const { return m_count; }
	
		private:
			uint m_count;
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
			ComputeBuffer(const BufferLayout& layout, uint count);
			~ComputeBuffer();
			void Resize(uint newCount);
			void ValidateSize(uint newCount);
			void MapBuffer(const void* data, size_t offset, size_t size);
			void MapBuffer(const void* data, size_t size);
			void SubmitData(const void* data, size_t offset, size_t size);
			
			void* BeginMapBuffer();
	
			template<typename T>
			Core::BufferView<T> BeginMapBuffer()
			{
				return { reinterpret_cast<T*>(BeginMapBuffer()), GetSize() / sizeof(T) };
			}
	
			void EndMapBuffer();
			
			size_t GetSize() const { return m_count * m_layout.GetStride(); }
			size_t GetStride() const { return m_layout.GetStride(); }
			size_t GetCount() const { return m_count; }
		private:
			BufferLayout m_layout;
			size_t m_count;
	};
}