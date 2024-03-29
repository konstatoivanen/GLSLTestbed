#pragma once
#include "PreCompiledHeader.h"
#include "Utilities/Ref.h"
#include "Core/AssetDataBase.h"
#include "Rendering/Objects/Buffer.h"
#include "Rendering/Structs/StructsCommon.h"

namespace PK::Rendering::Objects
{
	using namespace Utilities;
	using namespace Structs;

	class Mesh : public GraphicsObject, public Asset
	{
		friend void AssetImporters::Import(const std::string& filepath, Ref<Mesh>& mesh);
	
		public:
			Mesh();
			Mesh(const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer);
			~Mesh();
		
			void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
			void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);
			inline void SetSubMeshes(const std::initializer_list<IndexRange>& indexRanges) { m_indexRanges = indexRanges; }
			inline void SetSubMeshes(const std::vector<IndexRange>& indexRanges) { m_indexRanges = indexRanges; }
		
			inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_vertexBuffers; }
			inline const Ref<IndexBuffer>& GetIndexBuffer() const { return m_indexBuffer; }
			const IndexRange GetSubmeshIndexRange(int submesh) const;
			inline const uint GetSubmeshCount() const { return glm::max(1, (int)m_indexRanges.size()); }
			inline const BoundingBox& GetLocalBounds() const { return m_localBounds; }
			inline void SetLocalBounds(const BoundingBox& bounds) { m_localBounds = bounds; }
	
		private:
			uint32_t m_vertexBufferIndex = 0;
			std::vector<Ref<VertexBuffer>> m_vertexBuffers;
			Ref<IndexBuffer> m_indexBuffer;
			std::vector<IndexRange> m_indexRanges;
			BoundingBox m_localBounds;
	};
}