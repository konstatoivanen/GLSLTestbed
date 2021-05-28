#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Back

#include includes/Shadowmapping.glsl

#multi_compile SHADOW_BLUR_PASS0 SHADOW_BLUR_PASS1

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float3 vs_TEXCOORD0;

void main()
{
    uint layer;
    SHADOW_SET_VERTEX_STATE_ATTRIBUTES(in_POSITION0, in_TEXCOORD0, layer);
    vs_TEXCOORD0 = float3(in_TEXCOORD0, layer);
};

#pragma PROGRAM_FRAGMENT
in float3 vs_TEXCOORD0;
layout(location = 0) out float2 SV_Target0;

void main()
{
    SV_Target0 = SAMPLE_SRC(vs_TEXCOORD0);
};