#pragma once

namespace PKStructs
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
        float4 tangent;
        float2 texcoord;
    };
    
    struct PKLight
    {
        float4 color;
        float4 direction;
    };

    struct IndexRange
    {
        uint offset;
        uint count;
    };
}