#version 460
#pragma PROGRAM_COMPUTE

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/SceneGIShared.glsl

layout(rgba16) uniform writeonly image2D pk_SceneGI_DiffuseWrite;
layout(rgba16) uniform writeonly image2D pk_SceneGI_SpecularWrite;
uniform int2 pk_SceneGI_Checkerboard_Offset = int2(0,0);

void ReprojectNeighbours(int2 basecoord, int2 coord, int2 size)
{
	int2 coord0 = basecoord + ((pk_SceneGI_Checkerboard_Offset + int2(1)) % int2(2));
	int2 coord1 = int2(coord.x, coord0.y);
	int2 coord2 = int2(coord0.x, coord.y);

	float3 worldposition0 = SampleWorldPosition(coord0, size);
	float3 worldposition1 = SampleWorldPosition(coord1, size);
	float3 worldposition2 = SampleWorldPosition(coord2, size);

	float2 uv0 = ClipToUVW(mul(pk_MATRIX_L_VP, float4(worldposition0, 1.0f))).xy;
	float2 uv1 = ClipToUVW(mul(pk_MATRIX_L_VP, float4(worldposition1, 1.0f))).xy;
	float2 uv2 = ClipToUVW(mul(pk_MATRIX_L_VP, float4(worldposition2, 1.0f))).xy;

	imageStore(pk_SceneGI_DiffuseWrite, coord0, tex2D(pk_ScreenGI_Diffuse, uv0));
	imageStore(pk_SceneGI_SpecularWrite, coord0, tex2D(pk_ScreenGI_Specular, uv0));

	imageStore(pk_SceneGI_DiffuseWrite, coord1, tex2D(pk_ScreenGI_Diffuse, uv1));
	imageStore(pk_SceneGI_SpecularWrite, coord1, tex2D(pk_ScreenGI_Specular, uv1));

	imageStore(pk_SceneGI_DiffuseWrite, coord2, tex2D(pk_ScreenGI_Diffuse, uv2));
	imageStore(pk_SceneGI_SpecularWrite, coord2, tex2D(pk_ScreenGI_Specular, uv2));
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() 
{
	int2 size = imageSize(pk_SceneGI_DiffuseWrite).xy;
	int2 basecoord = int2(gl_GlobalInvocationID.xy) * 2;
	int2 coord = basecoord + pk_SceneGI_Checkerboard_Offset;

	if (any(Greater(coord, size)))
	{
		return;
	}

	float3 worldposition = SampleWorldPosition(coord, size);
	ReprojectNeighbours(basecoord, coord, size);

	if (Greater(abs(WorldToVoxelClipSpace(worldposition)), 1.0f.xxx))
	{
		imageStore(pk_SceneGI_DiffuseWrite, coord, float4(0.0f.xxx, 1.0f));
		imageStore(pk_SceneGI_SpecularWrite, coord, float4(0.0f.xxx, 1.0f));
		return;
	}

	// Find a base for the side cones with the normal as one of its base vectors.
	const float4 NR = SampleWorldSpaceNormalRoughness(coord);
	const float3 N = NR.xyz;
	const float3 O = worldposition;
	const float3 V = normalize(worldposition - pk_WorldSpaceCameraPos.xyz);
	const float3 R = reflect(V, N);
	const float3 D = GlobalNoiseBlue(uint2(coord + pk_Time.xy * 512)).xyz;

	imageStore(pk_SceneGI_DiffuseWrite, coord, ConeTraceDiffuse(O, N, D.x));
	imageStore(pk_SceneGI_SpecularWrite, coord, ConeTraceSpecular(O, N, R, D.y, NR.w));
}