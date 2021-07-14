#version 460
#pragma PROGRAM_COMPUTE

#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/SceneGIShared.glsl

layout(rgba16) uniform writeonly image2D pk_SceneGI_DiffuseWrite;
layout(rgba16) uniform writeonly image2D pk_SceneGI_SpecularWrite;
uniform uint pk_SceneGI_Checkerboard_Flip = 0;

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() 
{
	uint2 size = imageSize(pk_SceneGI_DiffuseWrite).xy;
	uint2 id = gl_GlobalInvocationID.xy;
	id.x *= 4u;
	id.x += (id.y + pk_SceneGI_Checkerboard_Flip) % 4;

	if (any(Greater(id, size)))
	{
		return;
	}

	float2 uv = id / float2(size);

	float3 worldposition = SampleWorldPosition(uv);

	if (Greater(abs(WorldToVoxelClipSpace(worldposition)), 1.0f.xxx))
	{
		imageStore(pk_SceneGI_DiffuseWrite, int2(id), float4(0.0f.xxx, 1.0f));
		imageStore(pk_SceneGI_SpecularWrite, int2(id), float4(0.0f.xxx, 1.0f));
		return;
	}

	// Find a base for the side cones with the normal as one of its base vectors.
	const float3 N = SampleWorldSpaceNormal(uv);
	const float3 O = worldposition;
	const float3 V = normalize(worldposition - pk_WorldSpaceCameraPos.xyz);
	const float3 R = reflect(V, N);
	const float3 D = GlobalNoiseBlueUV(uv * (size / 256.0f.xx) + pk_Time.xy).xyz;

	imageStore(pk_SceneGI_DiffuseWrite, int2(id), ConeTraceDiffuse(O, N, D.x));
	imageStore(pk_SceneGI_SpecularWrite, int2(id), ConeTraceSpecular(O, N, R, D.y, SampleRoughness(uv)));
}