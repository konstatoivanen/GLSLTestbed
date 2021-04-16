#version 460
#extension GL_ARB_bindless_texture : require

#Blend One SrcAlpha

#include includes/PKCommon.glsl
#include includes/Noise.glsl
#include includes/VolumeResources.glsl

struct Varyings
{
	float2 uv;
	float3 viewdir;
};

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD;

void main()
{
	vs_TEXCOORD = in_TEXCOORD0;
	gl_Position = in_POSITION0;
};

#pragma PROGRAM_FRAGMENT
float SampleDepth(float2 uv)
{
	return LinearizeDepth(tex2D(pk_ScreenDepth, uv).r);
}

in float2 vs_TEXCOORD;
layout(location = 0) out float4 SV_Target0;
void main() 
{
	float linearDepth = SampleDepth(vs_TEXCOORD);
	float linear01Depth = unlerp_sat(pk_ProjectionParams.y, pk_ProjectionParams.z, linearDepth);
	
	float3 offset = NoiseBlue(int2(vs_TEXCOORD * pk_ScreenParams.xy + pk_Time.ww * 1000));
	offset.xy -= 0.5f;
	offset.xy *= 4.0f / textureSize(pk_Volume_ScatterRead, 0).xy;
	offset.z  /= VOLUME_DEPTH;

	float3 uvw = float3(vs_TEXCOORD + offset.xy, linear01Depth - offset.z);

	SV_Target0 = tex2D(pk_Volume_ScatterRead, uvw);
};