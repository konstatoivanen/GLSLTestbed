#version 460

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/Encoding.glsl
#include includes/SceneGIShared.glsl

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

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;
layout(location = 1) out float4 SV_Target1;
void main() 
{
	float3 worldposition = SampleWorldPosition(vs_TEXCOORD0);

	if (Greater(abs(WorldToVoxelClipSpace(worldposition)), 1.0f.xxx))
	{
		SV_Target0 = float4(0.0f.xxx, 1.0f);
		SV_Target1 = float4(0.0f.xxx, 1.0f);
		return;
	}

	
	// Find a base for the side cones with the normal as one of its base vectors.
	const float3 N = SampleWorldSpaceNormal(vs_TEXCOORD0);
	const float3 O = worldposition;
	const float3 V = normalize(worldposition - pk_WorldSpaceCameraPos.xyz);
	const float3 R = reflect(V, N);
	const float3 D = GlobalNoiseBlueUV(vs_TEXCOORD0 * (pk_ScreenParams.xy / 256.0f) + pk_Time.xy).xyz;

	//SV_Target0 = SampleSceneGI(O, 0.0f);
	SV_Target0 = ConeTraceDiffuse(O, N, D.x);
	SV_Target1 = ConeTraceSpecular(O, N, R, D.y, SampleRoughness(vs_TEXCOORD0));
}