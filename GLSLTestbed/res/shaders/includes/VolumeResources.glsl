#pragma once
#include HLSLSupport.glsl

#define RAYMARCH_STEP_COUNT 64
#define VOLUME_DEPTH 128
#define VOLUME_WIDTH 160
#define VOLUME_HEIGHT 90
#define VOLUME_DEPTH_FLOAT 127.0f
#define VOLUME_SIZE_XY float2(159.0f, 89.0f)

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
};

layout(rgba16f) uniform image3D pk_Volume_Inject;
layout(rgba16f) uniform image3D pk_Volume_Scatter;
uniform sampler3D pk_Volume_ScatterRead;
