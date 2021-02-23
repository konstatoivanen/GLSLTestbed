#pragma once
#include "PrecompiledHeader.h"
#include "Rendering/Objects/GraphicsObject.h"
#include "Rendering/Structs/PropertyBlock.h"
#include "Rendering/Structs/BufferLayout.h"
#include <GL/glew.h>
#include <hlslmath.h>

class VertexBuffer : public GraphicsObject
{
	public:
		VertexBuffer(uint size);
		VertexBuffer(float* vertices, uint size);
		VertexBuffer(uint elementCount, const BufferLayout& layout);
		VertexBuffer(float* vertices, uint elementCount, const BufferLayout& layout);
		~VertexBuffer();

		void SetData(const void* data, uint size);
	
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