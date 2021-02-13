#pragma once
#include "Rendering\Mesh.h"
#include <hlslmath.h>

namespace MeshUtilities
{
    struct Vertex_Simple
    {
        float3 position;
        float2 texcoord;
    };
    
    struct Vertex_Full
    {
        float3 position;
        float3 normal;
        float2 texcoord;
    };

    Ref<Mesh> GetBoxSimple(const float3& offset, const float3& extents);
    Ref<Mesh> GetBox(const float3& offset, const float3& extents);
    Ref<Mesh> GetQuad2D(const float2& min, const float2& max);
    Ref<Mesh> GetQuad3D(const float2& min, const float2& max);
}
