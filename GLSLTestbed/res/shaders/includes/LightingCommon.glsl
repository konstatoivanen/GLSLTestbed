#pragma once
#include HLSLSupport.glsl

#define LIGHT_PARAM_INVALID 0xFFFFFFFF
#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_SPOT 1
#define LIGHT_TYPE_DIRECTIONAL 2

struct LightTile 
{
    uint start;
    uint end;
};

struct PKRawLight
{
    float4 color;
    float4 position;
    uint shadowmap_index;
    uint projection_index;
    uint cookie_index;
    uint type;
};

struct PKLight
{
    float3 color;
    float3 direction;
};

struct PKIndirect
{
    float3 diffuse;
    float3 specular;
};

struct SurfaceData
{
    float3 albedo;      
    float3 normal;      
    float3 emission;
    float metallic;     
    float roughness;
    float occlusion;
    float alpha;
};

PKLight EmptyLight()
{
    return PKLight(float3(0,0,0), float3(0,1,0));
}

PKIndirect EmptyIndirect()
{
    return PKIndirect(float3(0,0,0), float3(0,0,0));
}

LightTile CreateLightTile(uint data)
{
	uint offset = data & 0xFFFFFF;
	return LightTile(offset, offset + (data >> 24));
}

uniform int pk_LightCount;
PK_DECLARE_READONLY_BUFFER(PKRawLight, pk_Lights);
PK_DECLARE_READONLY_BUFFER(float4x4, pk_LightMatrices);

#if defined(PK_WRITE_CLUSTER_LIGHTS)
    PK_DECLARE_WRITEONLY_BUFFER(uint, pk_GlobalLightsList);
    layout(r32ui) uniform writeonly uimage3D pk_LightTiles;
#else
    PK_DECLARE_READONLY_BUFFER(uint, pk_GlobalLightsList);
    layout(r32ui) uniform readonly uimage3D pk_LightTiles;
#endif
