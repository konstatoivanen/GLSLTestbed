#pragma once
#include HLSLSupport.glsl

struct PKLight
{
    float4 color;
    float4 direction;
};

struct PKIndirect
{
    float4 diffuse;
    float4 specular;
};

struct PKGI
{
    PKLight light;
    PKIndirect indirect;
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
    return PKLight(float4(0,0,0,0), float4(0,1,0,0));
}

PKIndirect EmptyIndirect()
{
    return PKIndirect(float4(0,0,0,0), float4(0,0,0,0));
}

uniform int pk_LightCount;
PK_DECLARE_BUFFER(PKLight, pk_Lights);