#version 460
#extension GL_ARB_bindless_texture : require

#Blend One SrcAlpha

#include includes/VolumeFogShared.glsl

struct Varyings
{
	float2 uv;
	float3 viewdir;
};

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float4 vs_TEXCOORD;

void main()
{
	vs_TEXCOORD = float4(in_TEXCOORD0, in_TEXCOORD0 * pk_ScreenParams.xy + pk_Time.ww * 1000);
	gl_Position = in_POSITION0;
};

#pragma PROGRAM_FRAGMENT
float SampleDepth(float2 uv)
{
	return LinearizeDepth(tex2D(pk_ScreenDepth, uv).r);
}

in float4 vs_TEXCOORD;
layout(location = 0) out float4 SV_Target0;
void main() 
{
	float d = LinearizeDepth(tex2D(pk_ScreenDepth, vs_TEXCOORD.xy).r);
	float w = GetVolumeWCoord(d);

	float3 offset = GlobalNoiseBlue(int2(vs_TEXCOORD.zw));
	offset -= 0.5f;
	offset *= VOLUME_COMPOSITE_DITHER_AMOUNT;

	SV_Target0 = tex2D(pk_Volume_ScatterRead, float3(vs_TEXCOORD.xy, w) + offset.xyz);
};