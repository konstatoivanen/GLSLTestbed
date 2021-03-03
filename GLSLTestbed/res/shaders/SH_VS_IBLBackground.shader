#version 460

#include Lighting.glsl

#pragma PROGRAM_VERTEX

layout(location = 0) in float4 in_POSITION0;
out float3 vs_TEXCOORD0;

void main()
{
	gl_Position = in_POSITION0;
	vs_TEXCOORD0 = mul(pk_MATRIX_I_VP, float4(in_POSITION0.xy, 1.0f, 1.0f)).xyz;
};

#pragma PROGRAM_FRAGMENT

in float3 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	float2 reflUV = OctaUV(normalize(vs_TEXCOORD0));
	float3 color = SampleEnv(reflUV, sin(pk_Time.y) * 0.5f + 0.5f);
	SV_Target0 = float4(color, 1.0f);
};