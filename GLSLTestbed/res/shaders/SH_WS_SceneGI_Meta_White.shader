#version 460

#ColorMask None
#Cull Off
#Blend Off
#ZTest Off
#ZWrite Off

#multi_compile _ PK_ENABLE_INSTANCING

#include includes/Lighting.glsl
#include includes/SceneGIShared.glsl

#pragma PROGRAM_VERTEX

layout(location = 0) in float3 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;
out float3 vs_WOLRDPOSITION;
out float3 vs_NORMAL;

void main() 
{
	vs_WOLRDPOSITION = ObjectToWorldPos(in_POSITION0);
	vs_NORMAL = ObjectToWorldNormal(in_NORMAL);
	gl_Position = WorldToVoxelNDCSpace(vs_WOLRDPOSITION);
}

#pragma PROGRAM_FRAGMENT
in float3 vs_WOLRDPOSITION;
in float3 vs_NORMAL;

void main() 
{
	float3 clipuvw;

	if (!TryGetWorldToClipUVW(vs_WOLRDPOSITION, clipuvw))
	{
		return;
	}

	float3 color = 0.0f.xxx;

	LightTile tile = GetLightTile(clipuvw);

	for (uint i = tile.start; i < tile.end; ++i)
	{
		PKLight light = GetSurfaceLight(i, vs_WOLRDPOSITION, tile.cascade);
		color += light.color * light.shadow * max(0.0f, dot(light.direction, vs_NORMAL));
	}
	
	StoreSceneGI(vs_WOLRDPOSITION, float4(color, 1.0f));
}