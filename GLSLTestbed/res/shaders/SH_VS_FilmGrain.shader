#version 460
#include includes/PKCommon.glsl
#include includes/Noise.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float3 vs_TEXCOORD0;

void main()
{
    gl_Position = in_POSITION0;
    vs_TEXCOORD0 = float3(in_TEXCOORD0, mod(pk_Time.x, 10.0f));
};

#pragma PROGRAM_FRAGMENT

in float3 vs_TEXCOORD0;
layout(location = 0) out float3 SV_Target0;

void main()
{
    SV_Target0 = NoiseGrainColor(vs_TEXCOORD0.xy, vs_TEXCOORD0.z) * 10.0f;
};