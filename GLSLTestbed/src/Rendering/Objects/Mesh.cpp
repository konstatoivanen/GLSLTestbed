#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Graphics.h"
#include "Rendering/MeshUtility.h"
#include <glad/glad.h>
#include <hlslmath.h>
#include <tinyobjloader/tiny_obj_loader.h>

namespace PK::Rendering::Objects
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Structs;
	using namespace PK::Math;

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
		GraphicsAPI::SetVertexBuffer(vertexBuffer);
	
		const auto& layout = vertexBuffer->GetLayout();
	
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case CG_TYPE::FLOAT:
				case CG_TYPE::FLOAT2:
				case CG_TYPE::FLOAT3:
				case CG_TYPE::FLOAT4:
				case CG_TYPE::INT:
				case CG_TYPE::INT2:
				case CG_TYPE::INT3:
				case CG_TYPE::INT4:
				{
					glEnableVertexAttribArray(m_vertexBufferIndex);
					glVertexAttribPointer(m_vertexBufferIndex,
						Convert::Components(element.Type),
						Convert::BaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)element.Offset);
					++m_vertexBufferIndex;
					break;
				}
				case CG_TYPE::FLOAT3X3:
				case CG_TYPE::FLOAT4X4:
				{
					auto count = Convert::Components(element.Type);
	
					for (uint8_t i = 0; i < count; i++)
					{
						glEnableVertexAttribArray(m_vertexBufferIndex);
						glVertexAttribPointer(m_vertexBufferIndex,
							count,
							Convert::BaseType(element.Type),
							element.Normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(element.Offset + sizeof(float) * count * i));
						glVertexAttribDivisor(m_vertexBufferIndex, 1);
						++m_vertexBufferIndex;
					}
					break;
				}
				default:
					PK_CORE_ASSERT(false, "Unknown PK_BUFFER_DATA_TYPE!");
			}
		}
	
		m_vertexBuffers.push_back(vertexBuffer);
	}
	
	void Mesh::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_graphicsId);
		GraphicsAPI::SetIndexBuffer(indexBuffer);
		m_indexBuffer = indexBuffer;
	}
	
	const Structs::IndexRange Mesh::GetSubmeshIndexRange(int submesh) const
	{
		if (submesh < 0 || m_indexRanges.empty())
		{
			return { 0, m_indexBuffer->GetCount() };
		}
	
		auto idx = glm::min((uint)submesh, (uint)m_indexRanges.size());
		return m_indexRanges.at(idx);
	}
	
}

template<>
void PK::Core::AssetImporters::Import<PK::Rendering::Objects::Mesh>(const std::string& filepath, Ref<PK::Rendering::Objects::Mesh>& mesh)
{
	using namespace PK::Rendering::Objects;
	using namespace PK::Rendering::Structs;

	if (mesh->m_graphicsId)
	{
		glDeleteVertexArrays(1, &mesh->m_graphicsId);
	}

	glCreateVertexArrays(1, &mesh->m_graphicsId);

	mesh->m_vertexBufferIndex = 0;
	mesh->m_vertexBuffers.clear();
	mesh->m_indexBuffer = nullptr;
	mesh->m_indexRanges.clear();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	
	bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), Utilities::String::ReadDirectory(filepath).c_str(), true);

	PK_CORE_ASSERT(err.empty(), err.c_str());
	PK_CORE_ASSERT(!attrib.vertices.empty(), "Mesh doesn't contain vertices");
	PK_CORE_ASSERT(success, "Failed to load .obj");

	uint indexCount = 0;
	std::vector<uint> indices;
	std::vector<PK::Rendering::Structs::IndexRange> submeshes;

	for (size_t i = 0; i < shapes.size(); ++i) 
	{
		auto& tris = shapes.at(i).mesh.indices;
		auto tcount = (uint)tris.size();

		submeshes.push_back({ indexCount, tcount });

		for (uint j = 0; j < tcount; ++j)
		{
			indices.push_back(tris.at(j).vertex_index);
		}

		indexCount += tcount;
	}

	auto vcount = attrib.vertices.size() / 3;
	auto hasNormals = !attrib.normals.empty() && (attrib.normals.size() / 3) == vcount;
	auto hasUVs = !attrib.texcoords.empty() && (attrib.texcoords.size() / 2) == vcount;

	mesh->AddVertexBuffer(CreateRef<VertexBuffer>(attrib.vertices.data(), vcount, BufferLayout({ { CG_TYPE::FLOAT3, "POSITION" } })));

	if (hasNormals)
	{
		mesh->AddVertexBuffer(CreateRef<VertexBuffer>(attrib.normals.data(), vcount, BufferLayout({ { CG_TYPE::FLOAT3, "NORMAL" } })));
	}
	else
	{
		auto normals = PK_CONTIGUOUS_ALLOC(float3, vcount);
		auto vertices = reinterpret_cast<float3*>(attrib.vertices.data());
		PK::Rendering::MeshUtility::CalculateNormals(vertices, indices.data(), normals, (uint)vcount, (uint)indices.size());
		mesh->AddVertexBuffer(CreateRef<VertexBuffer>(normals, vcount, BufferLayout({ { CG_TYPE::FLOAT3, "NORMAL" } })));
		free(normals);
	}

	if (hasUVs)
	{
		mesh->AddVertexBuffer(CreateRef<VertexBuffer>(attrib.texcoords.data(), vcount, BufferLayout({ { CG_TYPE::FLOAT2, "TEXCOORD0" } })));
	}

	mesh->SetIndexBuffer(CreateRef<IndexBuffer>(indices.data(), (uint)indices.size()));
	mesh->SetSubMeshes(submeshes);
}