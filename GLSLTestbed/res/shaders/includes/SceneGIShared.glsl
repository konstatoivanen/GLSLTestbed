#pragma once
#include HLSLSupport.glsl

#define MAX_MIP_LEVEL 5.4f
#define VOXEL_SIZE pk_SceneGI_ST.w

layout(rgba16) uniform writeonly image3D pk_SceneGI_VolumeWrite;
uniform sampler3D pk_SceneGI_VolumeRead;
uniform float4 pk_SceneGI_ST;

int3 WorldToVoxelSpace(float3 worldposition)
{
    return int3((worldposition - pk_SceneGI_ST.xyz) / pk_SceneGI_ST.www); 
}

float3 WorldToSampleSpace(float3 worldposition)
{
    return ((worldposition - pk_SceneGI_ST.xyz) / pk_SceneGI_ST.www) / textureSize(pk_SceneGI_VolumeRead, 0).xyz; 
}

float3 WorldToClipSpace(float3 worldposition)
{
    return WorldToSampleSpace(worldposition) * 2.0f - 1.0f;
}

float4 SampleSceneGI(float3 worldposition, float level)
{
    float4 value = tex2DLod(pk_SceneGI_VolumeRead, WorldToSampleSpace(worldposition), min(MAX_MIP_LEVEL, level));
    value.rgb *= 16.0f;
    return value;
}

void StoreSceneGI(float3 worldposition, float3 color)
{
	imageStore(pk_SceneGI_VolumeWrite, WorldToVoxelSpace(worldposition), float4(color / 1024.0f, 1.0f));
}