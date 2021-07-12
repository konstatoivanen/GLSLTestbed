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
    uint cascade;
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

LightTile CreateLightTile(uint data)
{
	uint offset = bitfieldExtract(data, 0, 20);
	uint count = bitfieldExtract(data, 20, 8);
    uint cascade = bitfieldExtract(data, 28, 4);
    return LightTile(offset, offset + count, cascade);
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
