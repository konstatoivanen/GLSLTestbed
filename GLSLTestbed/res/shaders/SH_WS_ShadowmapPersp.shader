#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Back
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
	gl_Layer = int(indices.batchIndex);

	PKRawLight light = PK_BUFFER_DATA(pk_Lights, indices.lightIndex);
	float3 wpos = ObjectToWorldPos(in_POSITION0);
	vs_VIEWVECTOR = wpos - light.position.xyz;

	gl_Position = mul(PK_BUFFER_DATA(pk_LightMatrices, light.projection_index), float4(wpos, 1.0f));
	gl_ViewportIndex = 1;
};

#pragma PROGRAM_FRAGMENT
// Fragment will be forwarded from Shadowmapping.glsl