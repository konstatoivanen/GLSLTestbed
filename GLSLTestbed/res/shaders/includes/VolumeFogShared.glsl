#pragma once
#include PKCommon.glsl
#include Noise.glsl

#define VOLUME_DEPTH 128
#define VOLUME_INV_DEPTH 0.0078125f // 1.0f / 128.0f
#define VOLUME_WIDTH 160
#define VOLUME_HEIGHT 90
#define VOLUME_SIZE_ST float3(0.00625f, 0.0111111111111111f, 0.5f) // (1.0f / 160.0f, 1.0f / 90.0f, 0.5f)
#define VOLUME_COMPOSITE_DITHER_AMOUNT 2.0f * float3(0.00625f, 0.0111111111111111f, 0.0078125f)

#define VOLUME_ACCUMULATION_LD min(1.0f, 35.0f * max(pk_DeltaTime.x, 0.01f))
#define VOLUME_ACCUMULATION_SC min(1.0f, 14.0f * max(pk_DeltaTime.x, 0.01f))

PK_DECLARE_CBUFFER(pk_VolumeResources)
{
    float4 pk_Volume_WindDir;
    float pk_Volume_ConstantFog;
    float pk_Volume_HeightFogExponent;
    float pk_Volume_HeightFogOffset;
    float pk_Volume_HeightFogAmount;
    float pk_Volume_Density;
    float pk_Volume_Intensity;
    float pk_Volume_Anisotropy;
    float pk_Volume_NoiseFogAmount;
    float pk_Volume_NoiseFogScale;
    float pk_Volume_WindSpeed;
    sampler3D pk_Volume_ScatterRead;
};

layout(rgba16f) uniform image3D pk_Volume_Inject;
layout(rgba16f) uniform image3D pk_Volume_Scatter;

float GetVolumeCellDepth(float index)
{
    return pk_ProjectionParams.x * pow(pk_ExpProjectionParams.z, index / VOLUME_DEPTH);
}

float GetVolumeWCoord(float depth)
{
    return max(log2(depth) * pk_ExpProjectionParams.x + pk_ExpProjectionParams.y, 0.0);
}

float GetVolumeSliceWidth(int index)
{
    float2 nf = pk_ProjectionParams.xx * pow(pk_ExpProjectionParams.zz, float2(index, index + 1) * VOLUME_INV_DEPTH);
    return nf.y - nf.x;
}

float3 GetVolumeCellNoise(uint3 id)
{
    return GlobalNoiseBlue(id.xy + id.z * int2(VOLUME_WIDTH, VOLUME_HEIGHT) + int(pk_Time.w * 1000).xx);
}