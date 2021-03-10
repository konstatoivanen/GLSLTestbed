#version 460

#include includes/Test.glsl

#pragma PROGRAM_VERTEX
#define USE_VERTEX_PROGRAM_IMG
#include includes/BlitCommon.glsl

#pragma PROGRAM_FRAGMENT
in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	float2 uv = vs_TEXCOORD0;
	
	uv.x -= 0.5;
	uv.x *= pk_ScreenParams.x / pk_ScreenParams.y;
	uv.x += 0.5;

	float tof = pk_Time.y * 0.1;
	float rt = (sin(pk_Time.y * 0.125) + 1) * 0.5;
	float res = lerp(256, pk_ScreenParams.y, rt * rt);

	float2 uvLow = round(uv * res);
	uv = uvLow / res;
	m2s o = m2s(tof, (sin(pk_Time.y * 0.25) + 1) * 0.5, rt, res, round(uv * res), uvLow / res, abs(rotate2D(uv - 0.5, tof * 2)));

	float3 col = float3(0,0,0);
	const int iter = 16;
	float offset = 1.0;

	for (int i = 0; i < iter; i++)
	{
		o.uv = (uv - 0.5) * offset + 0.5;

		float4 ncol = sampleImage(o);
		col = lerp(col, ncol.rgb, ncol.a);

		offset -= 0.1;
	}

	SV_Target0 = float4(col, 1.0);
};