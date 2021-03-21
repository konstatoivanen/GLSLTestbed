#pragma once
#ifndef PK_NOISE
#define PK_NOISE

#include HLSLSupport.glsl

float NoiseUV(float u, float v)
{
    return fract(43758.5453 * sin(dot(float2(12.9898, 78.233), float2(u, v))));
}

float NoiseGradient(float2 uv, float2 res)
{
    return fract(52.9829189f * fract(dot(float2(0.06711056f, 0.00583715f), floor(uv * res))));
}

#endif