#version 460
#pragma PROGRAM_COMPUTE

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/SceneGIShared.glsl

layout(rgba16) uniform writeonly image2D pk_SceneGI_DiffuseWrite;
layout(rgba16) uniform writeonly image2D pk_SceneGI_SpecularWrite;
uniform int2 pk_SceneGI_Checkerboard_Offset = int2(0,0);

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() 
{
	int2 size = imageSize(pk_SceneGI_DiffuseWrite).xy;
	int2 id = int2(gl_GlobalInvocationID.xy) * 2 + pk_SceneGI_Checkerboard_Offset;

	if (any(Greater(id, size)))
	{
		return;
	}

	float3 worldposition = SampleWorldPosition(id, size);

	if (Greater(abs(WorldToVoxelClipSpace(worldposition)), 1.0f.xxx))
	{
		imageStore(pk_SceneGI_DiffuseWrite, id, float4(0.0f.xxx, 1.0f));
		imageStore(pk_SceneGI_SpecularWrite, id, float4(0.0f.xxx, 1.0f));
		return;
	}

	// Find a base for the side cones with the normal as one of its base vectors.
	const float3 N = SampleWorldSpaceNormal(id);
	const float3 O = worldposition;
	const float3 V = normalize(worldposition - pk_WorldSpaceCameraPos.xyz);
	const float3 R = reflect(V, N);
	const float3 D = GlobalNoiseBlue(uint2(id + pk_Time.xy * 512)).xyz;

	imageStore(pk_SceneGI_DiffuseWrite, id, ConeTraceDiffuse(O, N, D.x));
	imageStore(pk_SceneGI_SpecularWrite, id, ConeTraceSpecular(O, N, R, D.y, SampleRoughness(id)));
}