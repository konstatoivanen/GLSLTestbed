#pragma once
#ifndef PK_RECONSTRUCTION
#define PK_RECONSTRUCTION

#include PKCommon.glsl

float3 UnpackNormal(in float3 packedNormal)
{
    return packedNormal * 2.0f - float3(1.0f);
}

float3 SampleNormal(in sampler2D map, in float3x3 rotation, in float2 uv, float amount)
{
    float3 unpackedNormal =  UnpackNormal(tex2D(map, uv).xyz);
    return lerp(rotation[2], mul(rotation, unpackedNormal), amount);
}

float2 ParallaxOffset(float height, float heightAmount, float3 viewdir)
{
    return (height * heightAmount - heightAmount / 2.0f) * (viewdir.xy / (viewdir.z + 0.42f));
}

float3 SampleWorldSpaceNormal(float2 uv)
{
    float3 normal = tex2D(pk_ScreenNormals, uv).xyz;
    normal = normal * 2 - (NotEqual(normal, float3(0)) ? 1.0f : 0.0f);
    return normal;
}

float3 SampleViewSpaceNormal(float2 uv)
{
    return mul(float3x3(pk_MATRIX_V), SampleWorldSpaceNormal(uv));
}

#endif