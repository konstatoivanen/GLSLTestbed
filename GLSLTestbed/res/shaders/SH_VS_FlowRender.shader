#version 460

#include includes/PKCommon.glsl
#include includes/Encoding.glsl

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

layout(rg32ui) uniform readonly uimage2D pk_FlowTexture;

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	int2 coord = int2(vs_TEXCOORD0 * imageSize(pk_FlowTexture).xy);
	uint2 value = imageLoad(pk_FlowTexture, coord).xy;
	float grad = (value.x / 128.0f);

	float4 color = 1.0f.xxxx;

	color.rgb = HSVToRGB(float3(grad, 0.25f, 0.25f));
	color.rgb = lerp(0.0f.xxx, 1.0f.xxx, value.y);

	SV_Target0 = color;
};