#pragma once
#include PKCommon.glsl

PK_DECLARE_CBUFFER(pk_DofParams)
{
    float pk_FocalLength;
    float pk_FNumber;
    float pk_FilmHeight;
    float pk_FocusSpeed;
    float pk_MaximumCoC;
};

struct AutoFocusData
{
    float Distance;
    float LensCoefficient;
};

PK_DECLARE_ATOMIC_VARIABLE(AutoFocusData, pk_AutoFocusParams);

const int SAMPLE_COUNT = 22;
const float2 SAMPLE_KERNEL[SAMPLE_COUNT] =
{
    float2(0,0),
    float2(0.53333336,0),
    float2(0.3325279,0.4169768),
    float2(-0.11867785,0.5199616),
    float2(-0.48051673,0.2314047),
    float2(-0.48051673,-0.23140468),
    float2(-0.11867763,-0.51996166),
    float2(0.33252785,-0.4169769),
    float2(1,0),
    float2(0.90096885,0.43388376),
    float2(0.6234898,0.7818315),
    float2(0.22252098,0.9749279),
    float2(-0.22252095,0.9749279),
    float2(-0.62349,0.7818314),
    float2(-0.90096885,0.43388382),
    float2(-1,0),
    float2(-0.90096885,-0.43388376),
    float2(-0.6234896,-0.7818316),
    float2(-0.22252055,-0.974928),
    float2(0.2225215,-0.9749278),
    float2(0.6234897,-0.7818316),
    float2(0.90096885,-0.43388376),
};


float GetTargetDistance()
{
    float d = SampleLinearDepth(float2(0.5f, 0.5f));
    return min(d, pk_ProjectionParams.y - 1e-4f);
}

float GetLensCoefficient(float focusDistance)
{
    return pk_FocalLength * pk_FocalLength / (pk_FNumber * (focusDistance - pk_FocalLength) * pk_FilmHeight * 2);
}

float GetLensCoefficient() { return PK_ATOMIC_DATA(pk_AutoFocusParams).LensCoefficient; }
float GetFocusDistance() { return PK_ATOMIC_DATA(pk_AutoFocusParams).Distance; }

float GetCircleOfConfusion01(float linearDepth)
{
    AutoFocusData data = PK_ATOMIC_DATA(pk_AutoFocusParams);
    return min(1.0f, abs(linearDepth - data.Distance) * data.LensCoefficient / linearDepth / pk_MaximumCoC);
}

float4 GetCirclesOfConfusion(float4 linearDepths)
{
    AutoFocusData data = PK_ATOMIC_DATA(pk_AutoFocusParams);
    return clamp((linearDepths - data.Distance) * data.LensCoefficient / linearDepths, -pk_MaximumCoC, pk_MaximumCoC);
}
