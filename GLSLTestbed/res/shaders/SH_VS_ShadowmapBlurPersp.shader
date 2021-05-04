#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Back

#include includes/Shadowmapping.glsl

#multi_compile SHADOW_BLUR_PASS0 SHADOW_BLUR_PASS1

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out flat uint vs_SAMPLELAYER;
out float2[SAMPLE_COUNT] vs_TEXCOORDS;

void main()
{
    SHADOW_SET_VERTEX_STATE_ATTRIBUTES(in_POSITION0, in_TEXCOORD0, vs_SAMPLELAYER);

    float R = GET_SHADOW_BLUR_AMOUNT_SPOT;

    #pragma unroll SAMPLE_COUNT
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vs_TEXCOORDS[i] = in_TEXCOORD0 + DistributeHammersley2D(SAMPLES_HAMMERSLEY_2D[i], R);
    }
};

#pragma PROGRAM_FRAGMENT
in flat uint vs_SAMPLELAYER;
in float2[SAMPLE_COUNT] vs_TEXCOORDS;
layout(location = 0) out float2 SV_Target0;

void main()
{
    float2 A = float2(0.0f);

    #pragma unroll SAMPLE_COUNT
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        A += SAMPLE_SRC(vs_TEXCOORDS[i], vs_SAMPLELAYER);
    }

    SV_Target0 = A / SAMPLE_COUNT;
};