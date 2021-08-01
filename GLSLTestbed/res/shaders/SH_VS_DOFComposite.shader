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

#define _Source_TexelSize (1.0f / textureSize(_Foreground, 0).xy)

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
    float linearDepth = LinearizeDepth(tex2D(pk_ScreenDepth, vs_TEXCOORD0).r);
    float coc = GetCircleOfConfusion(linearDepth);

    float4 foreground = tex2D(pk_Foreground, vs_TEXCOORD0);
    float4 background = tex2D(pk_Background, vs_TEXCOORD0);
    
    float texely = 1.0f / textureSize(pk_Foreground, 0).y;
    background.a = smoothstep(texely, texely * 2.0f, coc);

    float3 color = lerp(background.rgb * background.a, foreground.rgb, foreground.a);

    SV_Target0 = float4(color, (1.0f - foreground.a) * (1.0f - background.a));
};