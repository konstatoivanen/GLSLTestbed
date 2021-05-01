#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Back

#include includes/Shadowmapping.glsl

#multi_compile SHADOW_BLUR_PASS0 SHADOW_BLUR_PASS1

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;
out flat uint vs_SAMPLELAYER;
out flat float3[SAMPLE_COUNT] vs_OFFSETS;

void main()
{
    SHADOW_SET_VERTEX_STATE_ATTRIBUTES(in_POSITION0, in_TEXCOORD0, vs_SAMPLELAYER);
    vs_TEXCOORD0 = in_TEXCOORD0;
    
    float R = GET_SHADOW_BLUR_AMOUNT_3D;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vs_OFFSETS[i] = DistributeHammersley3D(SAMPLES_HAMMERSLEY_3D[i], R);
    }
};

#pragma PROGRAM_FRAGMENT
in float2 vs_TEXCOORD0;
in flat uint vs_SAMPLELAYER;
in flat float3[SAMPLE_COUNT] vs_OFFSETS;
layout(location = 0) out float2 SV_Target0;

void main()
{
    float2 uv = vs_TEXCOORD0;
    float3 N = OctaDecode(uv);
    float3 U = abs(N.z) < 0.999f ? half3(0.0f, 0.0f, 1.0f) : half3(1.0f, 0.0f, 0.0f);
    float3 T = normalize(cross(U, N));
    float3 B = cross(N, T);
    float3 H = float3(0.0f);
    float2 A = float2(0.0f);

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        H = T * vs_OFFSETS[i].x + B * vs_OFFSETS[i].y + N * vs_OFFSETS[i].z;
        A += SAMPLE_SRC_OCT(H, vs_SAMPLELAYER);
    }

    SV_Target0 = A / SAMPLE_COUNT;
};