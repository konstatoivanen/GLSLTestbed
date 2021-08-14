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

layout(r32ui) uniform readonly uimage2D pk_FlowTexture;
uniform sampler2D pk_Maze;

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	int2 coord = int2(vs_TEXCOORD0 * imageSize(pk_FlowTexture).xy);
	uint value = imageLoad(pk_FlowTexture, coord).x;
	
	float gradient = 1.0f - clamp((0xFFFFFF - value.x) / 8192.0f, 0.0, 1.0f);
	float H = gradient * 3.0f + 0.6f;
	float S = lerp(0.0f, 0.8f, gradient);
	float V = lerp(0.05f, 0.8f, gradient * gradient);

	float4 color = 1.0f.xxxx;

	color.rgb = HSVToRGB(H,S,V);

	float4 offset = float4(0.25f, 0.25f, -0.25f, -0.25f) / textureSize(pk_Maze, 0).xyxy; 
		                                                                                         
	float border = 0.0f;                                                              
	border += tex2D(pk_Maze, vs_TEXCOORD0 + offset.xy).r;                           
	border += tex2D(pk_Maze, vs_TEXCOORD0 + offset.zw).r;                 
	border += tex2D(pk_Maze, vs_TEXCOORD0 + offset.xw).r;                           
	border += tex2D(pk_Maze, vs_TEXCOORD0 + offset.zy).r;                           
	border = smoothstep(border, 0.9f, 0.25f);                                          
	border *= border;                                                                 
	border *= 0.75f;                                                                  

	color.rgb = lerp(color.rgb, 1.0f.xxx, border);

	SV_Target0 = color;
};