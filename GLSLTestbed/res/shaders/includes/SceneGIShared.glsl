#pragma once
#include HLSLSupport.glsl

#define VOXEL_SIZE pk_SceneGI_ST.w
#define PK_GI_ANGLE 5.08320368996f

layout(rgba16) uniform image3D pk_SceneGI_VolumeWrite;
uniform sampler3D pk_SceneGI_VolumeRead;
uniform float4 pk_SceneGI_ST;
uniform uint3 pk_GIVoxelAxisSwizzle;

int3 WorldToVoxelSpace(float3 worldposition) { return int3((worldposition - pk_SceneGI_ST.xyz) / pk_SceneGI_ST.www); }

float3 WorldToSampleSpace(float3 worldposition) { return ((worldposition - pk_SceneGI_ST.xyz) / pk_SceneGI_ST.www) / textureSize(pk_SceneGI_VolumeRead, 0).xyz;  }

float3 WorldToVoxelClipSpace(float3 worldposition) { return WorldToSampleSpace(worldposition) * 2.0f - 1.0f; }

float4 WorldToVoxelNDCSpace(float3 worldposition) 
{ 
	float3 clippos = WorldToVoxelClipSpace(worldposition);
	return float4(clippos[pk_GIVoxelAxisSwizzle.x], clippos[pk_GIVoxelAxisSwizzle.y], clippos[pk_GIVoxelAxisSwizzle.z] * 0.5f + 0.5f, 1);
}

float4 SampleSceneGI(float3 worldposition, float level)
{
    float4 value = tex2DLod(pk_SceneGI_VolumeRead, WorldToSampleSpace(worldposition), level);
    value.rgb *= 128.0f;
    return value;
}

void StoreSceneGI(float3 worldposition, float4 color) 
{ 
	int3 coord = WorldToVoxelSpace(worldposition);

	float4 value0 = imageLoad(pk_SceneGI_VolumeWrite, coord);
	float4 value1 = float4(color.rgb / 128.0f, color.a);

	value1.rgb = lerp(value0.rgb, value1.rgb, 1.0f - 0.9f * value0.a);

	imageStore(pk_SceneGI_VolumeWrite, coord, value1); 
}

float3 GetSampleDirectionSE(float3 worldNormal, uint index, const float sampleCount, float dither)
{
    float fi = float(index) + dither;
	float fiN = fi / sampleCount;
	float longitude = PK_GI_ANGLE * fi;
	float latitude = asin(fiN * 2.0 - 1.0);

	float3 kernel;
	kernel.x = cos(latitude) * cos(longitude);
	kernel.z = cos(latitude) * sin(longitude);
	kernel.y = sin(latitude);
	kernel = faceforward(kernel, kernel, -worldNormal); 

	return normalize(kernel);
}

float4 ConeTraceDiffuse(float3 origin, const float3 normal, const float dither) 
{
	float4 A = 0.0.xxxx;

	origin += normal * (1.0 + PK_INV_SQRT2) * VOXEL_SIZE;

	#pragma unroll 8
	for (uint i = 0u; i < 8u; ++i)
	{
		const float3 direction = GetSampleDirectionSE(normal, i, 8u, dither);

		float4 color = float4(0.0.xxx, 1.0);

		float dist = 1.0;

		#pragma unroll 4
		for (uint i = 0; i < 4; ++i)
		{
			float level = log2(dist / VOXEL_SIZE);
			float4 voxel = SampleSceneGI(origin + dist * direction, level);

			color.rgb += voxel.rgb * (1.0 + level) * pow2(color.a) * i;
			color.a *= saturate(1.0 - voxel.a * (1.0 + pow3(level) * 0.1));

			dist += pow2(level + 1) * VOXEL_SIZE;
		}

		A += color * max(0.0, dot(normal, direction));
	}

	A.a *= max(0.0, dot(normal, float3(0.0,1.0,0.0)) * 0.5 + 0.5);
	A /= 8.0;

	return A;
}

float4 ConeTraceSpecular(float3 origin, const float3 normal, const float3 direction, const float dither, float roughness) 
{
	origin += normal * (1.0f + PK_INV_SQRT2) * VOXEL_SIZE;

	float4 color = float4(0.0f.xxx, 1.0f);

	float dist = 0.0f;

	float conesize = pow2(roughness) * 2.5f;

	for (uint i = 0; i < 64; ++i)
	{
		float level = i * conesize;

		float4 voxel = SampleSceneGI(origin + dist * direction, level);

		color.rgb += voxel.rgb * voxel.a * color.a;
		color.a *= 1.0f - voxel.a;

		dist += VOXEL_SIZE * (1.0f + 0.125f * level);
	}

	return color;
}