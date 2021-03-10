#pragma once
#include "Rendering/Objects/Mesh.h"
#include <hlslmath.h>

namespace MeshUtilities
{
    void CalculateNormals(const float3* vertices, const uint* indices, float3* normals, uint vcount, uint icount);
    void CalculateTangents(const float3* vertices, const float3* normals, const float2* texcoords, const uint* indices, float4* tangents, uint vcount, uint icount);
    void CalculateTangents(void* vertices, uint stride, uint vertexOffset, uint normalOffset, uint tangentOffset, uint texcoordOffset, const uint* indices, uint vcount, uint icount);
    Ref<Mesh> GetBoxSimple(const float3& offset, const float3& extents);
    Ref<Mesh> GetBox(const float3& offset, const float3& extents);
    Ref<Mesh> GetQuad2D(const float2& min, const float2& max);
    Ref<Mesh> GetQuad3D(const float2& min, const float2& max);
    Ref<Mesh> GetSphere(const float3& offset, const float radius);
}
