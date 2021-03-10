#version 460

#ColorMask RGBA

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
	uv.x *= (pk_ScreenParams.x / pk_ScreenParams.y);
	uv.x += 0.5;

	float tof = pk_Time.y * 0.1;
	float st = (sin(pk_Time.y * 0.5) + 1) * 0.5;
	float rt = (sin(pk_Time.y * 0.25) + 1) * 0.5;
	float res = mix(128, pk_ScreenParams.y, rt * rt);

	float2 uvLow = round(uv * res);

	uv = uvLow / res;

	float2 a = abs(rotate2D(uv - 0.5, tof * 2));

	float box = max(a.x, a.y) - tof;
	float boxm = min(a.x, a.y) - tof;
	float circle = length(uv - 0.5) - tof;

	float shape = mix(mix(box, circle, saturate(st * 2)), boxm, saturate((st * 2) - 1));

	float hue = mod(shape, 1.0);
	float cut = step(mod(shape * 16.0, 1.0), 0.5);
	float grad = dither(mod(shape * 3, 1.0) * cut, uvLow, 4);

	float3 col = hsv2rgb(float3(hue, 1, grad));

	SV_Target0 = float4(col, 1.0);
};