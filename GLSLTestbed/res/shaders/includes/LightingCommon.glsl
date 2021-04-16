#pragma once
#include HLSLSupport.glsl

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
    uint shadowmap_proj_index;
    uint light_cookie_index;
    uint light_type;
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

uniform int pk_LightCount;
PK_DECLARE_READONLY_BUFFER(PKRawLight, pk_Lights);

#if defined(PK_WRITE_CLUSTER_LIGHTS)
    PK_DECLARE_WRITEONLY_BUFFER(uint, pk_GlobalLightsList);
    PK_DECLARE_WRITEONLY_BUFFER(uint, pk_LightTiles);
#else
    PK_DECLARE_READONLY_BUFFER(uint, pk_GlobalLightsList);
    PK_DECLARE_READONLY_BUFFER(uint, pk_LightTiles);
#endif
