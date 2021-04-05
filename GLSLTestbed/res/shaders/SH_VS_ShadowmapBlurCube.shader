#version 460

#include includes/PKCommon.glsl
#include includes/Lighting.glsl

#multi_compile BLUR_PASS0 BLUR_PASS1

#pragma PROGRAM_VERTEX
#define USE_VERTEX_PROGRAM_IMG
#include includes/BlitCommon.glsl

#pragma PROGRAM_FRAGMENT

#if defined(BLUR_PASS0)
uniform highp samplerCube _MainTex;
#else
uniform highp sampler2D _MainTex;
#endif

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

half2 Hammersley(uint i, uint N)
{
    return half2(float(i) / float(N), RadicalInverse_VdC(i));
}

half3 ImportanceSampleGGX(half2 Xi, half3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0f * 3.14159265f * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    half3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    half3 up = abs(N.z) < 0.999f ? half3(0.0f, 0.0f, 1.0f) : half3(1.0f, 0.0f, 0.0f);
    half3 tangent = normalize(cross(up, N));
    half3 bitangent = cross(N, tangent);

    half3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

in float2 vs_TEXCOORD0;
layout(location = 0) out float2 SV_Target0;

void main()
{
    const uint SAMPLE_COUNT = 16u;
    
    float3 N = OctaDecode(vs_TEXCOORD0);
    float3 V = N;
    float2 fsout = float2(0.0f);

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        half2 Xi = Hammersley(i, SAMPLE_COUNT);
        half3 H = ImportanceSampleGGX(Xi, N, 0.05f);
        half3 L = normalize(2.0f * dot(V, H) * H - V);

        #if defined(BLUR_PASS0)
            fsout += tex2D(_MainTex, L).rg;
        #else
            fsout += tex2D(_MainTex, OctaUV(L)).rg;
        #endif
    }

    SV_Target0 = fsout / SAMPLE_COUNT;
};