#pragma once
#include HLSLSupport.glsl

#define RAYMARCH_STEP_COUNT 64
#define VOLUME_DEPTH 192
#define VOLUME_WIDTH 160
#define VOLUME_HEIGHT 90
#define VOLUME_DEPTH_FLOAT 191.0f
#define VOLUME_WIDTH_FLOAT 159.0f
#define VOLUME_HEIGHT_FLOAT 89.0f

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
uniform sampler3D pk_Volume_InjectRead;
