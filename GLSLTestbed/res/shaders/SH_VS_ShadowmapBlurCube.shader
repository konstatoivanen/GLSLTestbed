#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Back

#include includes/PKCommon.glsl
#include includes/Lighting.glsl

#multi_compile BLUR_PASS0 BLUR_PASS1

#define SAMPLE_COUNT 16u

#pragma PROGRAM_VERTEX
// Original functions kept for reference.
/*
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

half2 Hammersley(uint i, uint N) { return half2(float(i) / float(N), RadicalInverse_VdC(i)); }

half3 ImportanceSampleGGX(half2 Xi, half3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * 3.14159265 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    half3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    half3 up = abs(N.z) < 0.999 ? half3(0.0, 0.0, 1.0) : half3(1.0, 0.0, 0.0);
    half3 tangent = normalize(cross(up, N));
    half3 bitangent = cross(N, tangent);

    half3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}
*/

const float3 SAMPLES_HAMMERSLEY[SAMPLE_COUNT] =
{
    float3(1.0f,            0.0f,           0.0f),
    float3(0.9238795f,      0.3826835f,     0.5f),
    float3(0.7071068f,      0.7071068f,     0.25f),
    float3(0.3826834f,      0.9238795f,     0.75f),
    float3(-4.371139E-08f,  1.0f,           0.125f),
    float3(-0.3826835f,     0.9238795f,     0.625f),
    float3(-0.7071068f,     0.7071068f,     0.375f),
    float3(-0.9238796f,     0.3826833f,     0.875f),
    float3(-1.0f,           -8.742278E-08f, 0.0625f),
    float3(-0.9238795f,     -0.3826834f,    0.5625f),
    float3(-0.7071066f,     -0.7071069f,    0.3125f),
    float3(-0.3826831f,     -0.9238797f,    0.8125f),
    float3(1.192488E-08f,   -1.0f,          0.1875f),
    float3(0.3826836f,      -0.9238794f,    0.6875f),
    float3(0.707107f,       -0.7071065f,    0.4375f),
    float3(0.9238796f,      -0.3826834f,    0.9375f),
};

half3 ImportanceSample(half3 Xi, float blur)
{
    float theta = (1.0f - Xi.z) / (1.0f + (blur - 1.0f) * Xi.z);
    float2 sincos = sqrt(float2(1.0f - theta, theta));
    return float3(Xi.xy * sincos.xx, sincos.y);
}

layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out flat uint vs_SAMPLELAYER;
out flat float3[SAMPLE_COUNT] vs_IMPORTANCESAMPLES;
out float2 vs_TEXCOORD0;

void main()
{
#if defined(BLUR_PASS0)
    gl_Layer = gl_InstanceID;
#else
    gl_ViewportIndex = gl_InstanceID;
#endif

    gl_Position = in_POSITION0;
    vs_TEXCOORD0 = in_TEXCOORD0;
    vs_SAMPLELAYER = gl_InstanceID;

    // Blur amount
    float R = pow5(0.125f);

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vs_IMPORTANCESAMPLES[i] = normalize(ImportanceSample(SAMPLES_HAMMERSLEY[i], R));
    }
};

#pragma PROGRAM_FRAGMENT

#if defined(BLUR_PASS0)
    uniform highp samplerCubeArray _MainTex;
    #define SAMPLE_SRC(H, layer) tex2D(_MainTex, float4(H, layer)).rg
#else
    uniform highp sampler2DArray _MainTex;
    #define SAMPLE_SRC(H, layer) tex2D(_MainTex, float3(OctaUV(H), vs_SAMPLELAYER)).rg;
#endif

in flat uint vs_SAMPLELAYER;
in flat float3[SAMPLE_COUNT] vs_IMPORTANCESAMPLES;
in float2 vs_TEXCOORD0;
layout(location = 0) out float2 SV_Target0;

void main()
{
    float2 uv = vs_TEXCOORD0;

    #if defined(BLUR_PASS1)
        uv *= float2(1.0f) + 2.0f / SHADOWMAP_TILE_SIZE;
        uv -= 1.0f / SHADOWMAP_TILE_SIZE;
        uv = saturate(uv);
    #endif
    
    float3 N = OctaDecode(uv);
    float3 U = abs(N.z) < 0.999f ? half3(0.0f, 0.0f, 1.0f) : half3(1.0f, 0.0f, 0.0f);
    float3 T = normalize(cross(U, N));
    float3 B = cross(N, T);
    float3 H = float3(0.0f);
    float2 A = float2(0.0f);

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        H = T * vs_IMPORTANCESAMPLES[i].x + B * vs_IMPORTANCESAMPLES[i].y + N * vs_IMPORTANCESAMPLES[i].z;
        A += SAMPLE_SRC(H, vs_SAMPLELAYER);
    }

    SV_Target0 = A / SAMPLE_COUNT;
};