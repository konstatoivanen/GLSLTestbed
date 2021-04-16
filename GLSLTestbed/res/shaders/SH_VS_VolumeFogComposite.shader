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

float4 ScatterStep(float3 accumulatedLight, float accumulatedTransmittance, float sliceDepth, float3 sliceLight, float sliceDensity)
{
	sliceDensity = max(sliceDensity, 0.000001);
	float sliceTransmittance = exp(-sliceDensity * sliceDepth);

	float3 sliceLightIntegral = sliceLight * (1.0 - sliceTransmittance) / sliceDensity;

	accumulatedLight += sliceLightIntegral * accumulatedTransmittance;
	accumulatedTransmittance *= sliceTransmittance;

	return float4(accumulatedLight, accumulatedTransmittance);
}

in float2 vs_TEXCOORD;
layout(location = 0) out float4 SV_Target0;
void main() 
{
	float3 offset = NoiseBlue(int2(vs_TEXCOORD * pk_ScreenParams.xy + pk_Time.ww * 1000));
	offset.x -= 0.5f;
	offset.y -= 0.5f;
	offset.xy *= 4.0f / textureSize(pk_Volume_InjectRead, 0).xy;

	float linearDepth = SampleDepth(vs_TEXCOORD);
	float linear01Depth = unlerp_sat(pk_ProjectionParams.y, pk_ProjectionParams.z, linearDepth);
	float depthSlice = linear01Depth / RAYMARCH_STEP_COUNT;

	float4 accum = float4(0, 0, 0, 1);
	float3 pos = float3(vs_TEXCOORD + offset.xy, offset.z * depthSlice);

	for (uint z = 0; z < RAYMARCH_STEP_COUNT; ++z)
	{
		float4 slice = tex2D(pk_Volume_InjectRead, pos);
		accum = ScatterStep(accum.rgb, accum.a, depthSlice, slice.rgb, slice.a);
		pos.z += depthSlice;
	}

	SV_Target0 = accum;
};