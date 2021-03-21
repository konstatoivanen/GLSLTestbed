#pragma once

namespace PK::Rendering::Structs
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
    
    struct PKPointLight
    {
        float4 color;
        float4 position;
    };

    struct IndexRange
    {
        uint offset;
        uint count;
    };
}