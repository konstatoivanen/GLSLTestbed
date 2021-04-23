#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Front
#ZTest LEqual
#ZWrite On

#multi_compile _ PK_ENABLE_INSTANCING

#define DRAW_SHADOW_MAP_FRAGMENT
#include includes/Shadowmapping.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float3 in_POSITION0;
out noperspective float3 vs_VIEWVECTOR;

void main()
{
	DrawIndices indices = ParseDrawIndices(PK_INSTANCE_PROPERTIES_ID);
	gl_Layer = int(indices.batchIndex * 6 + indices.faceIndex);

	float4 light = PK_BUFFER_DATA(pk_Lights, indices.lightIndex).position;
	float3 wpos = ObjectToWorldPos(in_POSITION0);
	vs_VIEWVECTOR = wpos - light.xyz;

	gl_Position = GetCubeClipPos(vs_VIEWVECTOR, light.w, indices.faceIndex);
	gl_ViewportIndex = 0;
};

#pragma PROGRAM_FRAGMENT
// Fragment will be forwarded from Shadowmapping.glsl