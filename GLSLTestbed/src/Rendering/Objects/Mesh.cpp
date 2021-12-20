#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/Mesh.h"
#include "Rendering/GraphicsAPI.h"
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
	
		// @TODO Refactor to use shared vertex array objects for buffer layouts
		glBindVertexArray(m_graphicsId);
		GraphicsAPI::SetVertexBuffer(vertexBuffer.get());
	
		const auto& layout = vertexBuffer->GetLayout();
	
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case PK_TYPE::FLOAT:
				case PK_TYPE::FLOAT2:
				case PK_TYPE::FLOAT3:
				case PK_TYPE::FLOAT4:
				case PK_TYPE::INT:
				case PK_TYPE::INT2:
				case PK_TYPE::INT3:
				case PK_TYPE::INT4:
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
				case PK_TYPE::FLOAT3X3:
				case PK_TYPE::FLOAT4X4:
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
		GraphicsAPI::SetIndexBuffer(indexBuffer.get());
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
bool PK::Core::AssetImporters::IsValidExtension<PK::Rendering::Objects::Mesh>(const std::filesystem::path& extension) { return extension.compare(".mdl") == 0; }

template<>
void PK::Core::AssetImporters::Import(const std::string& filepath, Ref<PK::Rendering::Objects::Mesh>& mesh)
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
	PK_CORE_ASSERT(!attrib.normals.empty(), "Mesh doesn't contain normals");
	PK_CORE_ASSERT(!attrib.texcoords.empty(), "Mesh doesn't contain uvs");
	PK_CORE_ASSERT(success, "Failed to load .obj");

	uint indexCount = 0;
	std::vector<uint> indices;
	std::vector<PK::Rendering::Structs::IndexRange> submeshes;
	std::vector<PK::Rendering::Structs::Vertex_Full> vertices;
	float3 minpos =  PK_FLOAT3_ONE * std::numeric_limits<float>().max();
	float3 maxpos = -PK_FLOAT3_ONE * std::numeric_limits<float>().max();

	auto invertices = attrib.vertices.data();
	auto innormals = attrib.normals.data();
	auto inuvs = attrib.texcoords.data();

	auto index = 0;

	for (size_t i = 0; i < shapes.size(); ++i) 
	{
		auto& tris = shapes.at(i).mesh.indices;
		auto tcount = (uint)tris.size();

		submeshes.push_back({ indexCount, tcount });

		for (uint j = 0; j < tcount; ++j)
		{
			auto& tri = tris.at(j);

			indices.push_back(index++);

			PK::Rendering::Structs::Vertex_Full v;
			v.position = *reinterpret_cast<float3*>(invertices + tri.vertex_index * 3);
			v.normal = *reinterpret_cast<float3*>(innormals + tri.normal_index * 3);
			v.tangent = PK_FLOAT4_ZERO;
			v.texcoord = *reinterpret_cast<float2*>(inuvs + tri.texcoord_index * 2);
			vertices.push_back(v);

			maxpos = glm::max(v.position, maxpos);
			minpos = glm::min(v.position, minpos);
		}

		indexCount += tcount;
	}


	BufferLayout layout = { {PK_TYPE::FLOAT3, "POSITION"}, {PK_TYPE::FLOAT3, "NORMAL"}, {PK_TYPE::FLOAT4, "TANGENT"}, {PK_TYPE::FLOAT2, "TEXCOORD0"} };

	PK::Rendering::MeshUtility::CalculateTangents(reinterpret_cast<float*>(vertices.data()), layout.GetStride() / 4, 0, 3, 6, 10, indices.data(), (uint)vertices.size(), (uint)indices.size());

	mesh->SetLocalBounds(PK::Math::Functions::CreateBoundsMinMax(minpos, maxpos));
	mesh->AddVertexBuffer(CreateRef<VertexBuffer>(vertices.data(), vertices.size(), layout, true));
	mesh->SetIndexBuffer(CreateRef<IndexBuffer>(indices.data(), (uint)indices.size(), true));
	mesh->SetSubMeshes(submeshes);
}