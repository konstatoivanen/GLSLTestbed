#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Mesh.h"
#include <GL\glew.h>
#include <hlslmath.h>

Mesh::Mesh()
{
	glCreateVertexArrays(1, &m_graphicsId);
}

Mesh::Mesh(const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer) : Mesh()
{
	AddVertexBuffer(vertexBuffer);
	SetIndexBuffer(indexBuffer);
}


Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &m_graphicsId);
}

void Mesh::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
{
	PK_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

	glBindVertexArray(m_graphicsId);
	vertexBuffer->Bind();

	const auto& layout = vertexBuffer->GetLayout();

	for (const auto& element : layout)
	{
		switch (element.Type)
		{
			case CG_TYPE_FLOAT:
			case CG_TYPE_FLOAT2:
			case CG_TYPE_FLOAT3:
			case CG_TYPE_FLOAT4:
			case CG_TYPE_INT:
			case CG_TYPE_INT2:
			case CG_TYPE_INT3:
			case CG_TYPE_INT4:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					CGType::Components(element.Type),
					CGType::BaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case CG_TYPE_FLOAT3X3:
			case CG_TYPE_FLOAT4X4:
			{
				auto count = CGType::Components(element.Type);

				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						CGType::BaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(element.Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				PK_CORE_ASSERT(false, "Unknown PK_BUFFER_DATA_TYPE!");
		}
	}

	m_VertexBuffers.push_back(vertexBuffer);
}

void Mesh::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
{
	glBindVertexArray(m_graphicsId);
	indexBuffer->Bind();
	m_IndexBuffer = indexBuffer;
}
