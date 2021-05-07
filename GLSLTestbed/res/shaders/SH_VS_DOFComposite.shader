#version 460
#Blend One SrcAlpha
#include includes/DepthOfFieldShared.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
    gl_Position = in_POSITION0;
    vs_TEXCOORD0 = in_TEXCOORD0;
};

#pragma PROGRAM_FRAGMENT

#define _MainTex_TexelSize (1.0f / textureSize(_MainTex, 0).xy)
#define EPSILON 0.0001f

uniform sampler2DArray _MainTex;

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
    float depth01 = tex2D(pk_ScreenDepth, vs_TEXCOORD0).r;
    float linearDepth = LinearizeDepth(depth01);

    const int2 OFFS0[4] = { int2(-1, 0), int2( 1,0), int2(0,1), int2(0,-1) };
    const int2 OFFS1[4] = { int2(-1,-1), int2(-1,1), int2(1,1), int2(1,-1) };

    // @TODO Could maybe optimize this by having a half res depth texture instead...?
    float4 Z0 = textureGatherOffsets(pk_ScreenDepth, vs_TEXCOORD0, OFFS0);
    float4 Z1 = textureGatherOffsets(pk_ScreenDepth, vs_TEXCOORD0, OFFS1);

    float4 depths = LinearizeDepth(float4(max(max(max(depth01, Z0.x), Z0.w), Z1.x),
                                          max(max(max(depth01, Z0.x), Z0.z), Z1.y),
                                          max(max(max(depth01, Z0.y), Z0.z), Z1.z),
                                          max(max(max(depth01, Z0.y), Z0.w), Z1.w)));
    
    float4 weights = float4(1.0f / (EPSILON + abs(linearDepth.xxxx - depths)));
    float weight = dot(weights, float4(1.0f));

    float3 uvw = float3(vs_TEXCOORD0, 1);
    
    float4 color  = textureOffset(_MainTex, uvw, int2(-1, -1)) * weights.x;
           color += textureOffset(_MainTex, uvw, int2(-1,  1)) * weights.y;
           color += textureOffset(_MainTex, uvw, int2( 1,  1)) * weights.z;
           color += textureOffset(_MainTex, uvw, int2( 1, -1)) * weights.w;
           color /= weight;

    float coc = GetCircleOfConfusion01(linearDepth);

    SV_Target0 = lerp(color, tex2D(_MainTex, uvw), coc);
};