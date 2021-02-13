#version 460

#Blend SrcAlpha OneMinusSrcAlpha
#ZTest LEqual
#ZWrite On
#Cull Back

#include PKCommon.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;
layout(location = 2) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
	gl_Position = ObjectToClipPos(in_POSITION0);
	vs_TEXCOORD0 = in_TEXCOORD0;
};

#pragma PROGRAM_FRAGMENT
in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	SV_Target0 = float4(vs_TEXCOORD0, 1, 1);
};