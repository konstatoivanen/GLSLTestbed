#pragma once
#include "Rendering/Objects/GraphicsObject.h"
#include "Rendering/Structs/BufferLayout.h"
#include "Rendering/Structs/PropertyBlock.h"
#include <glad/glad.h>
#include <hlslmath.h>

class VertexBuffer : public GraphicsObject
{
	public:
		VertexBuffer(size_t size);
		VertexBuffer(float* vertices, size_t size);
		VertexBuffer(size_t elementCount, const BufferLayout& layout);
		VertexBuffer(float* vertices, size_t elementCount, const BufferLayout& layout);
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
		void SetData(const void* data, size_t offset, size_t size);
		void SetData(const void* data, size_t size);
	private:
		BufferLayout m_layout;
};