#ifndef PK_Lighting
#define PK_Lighting

#include HLSLSupport.glsl

uniform float4 pk_SHAr;
uniform float4 pk_SHAg;
uniform float4 pk_SHAb;
uniform float4 pk_SHBr;
uniform float4 pk_SHBg;
uniform float4 pk_SHBb;
uniform float4 pk_SHC;

float3 SampleSphericalHarmonics(float4 normal)
{
    float3 x0, x1, x2;

    x0.r = dot(pk_SHAr, normal);
    x0.g = dot(pk_SHAg, normal);
    x0.b = dot(pk_SHAb, normal);

    float4 vB = normal.xyzz * normal.yzzx;
    x1.r = dot(pk_SHBr,vB);
    x1.g = dot(pk_SHBg,vB);
    x1.b = dot(pk_SHBb,vB);

    float vC = normal.x * normal.x - normal.y * normal.y;
    x2 = pk_SHC.rgb * vC;

    return x0 + x1 + x2;
}
#endif