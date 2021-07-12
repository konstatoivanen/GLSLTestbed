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
out float4 vs_DEPTH;

void main()
{
	DrawIndices indices = ParseDrawIndices(PK_INSTANCE_PROPERTIES_ID);
	gl_Layer = int(indices.faceIndex);

	PKRawLight light = PK_BUFFER_DATA(pk_Lights, indices.lightIndex);

	float3 wpos = ObjectToWorldPos(in_POSITION0);
	
	float4x4 lightmatrix = PK_BUFFER_DATA(pk_LightMatrices, light.projection_index + indices.faceIndex);

	gl_Position = mul(lightmatrix, float4(wpos, 1.0f));

	float depth = ((gl_Position.z / gl_Position.w) + 1.0f) * light.position.w * 0.5f;
	vs_DEPTH = float4(depth * light.position.xyz, SHADOW_NEAR_BIAS * (gl_Layer + 1));

	gl_ViewportIndex = 1;
};

#pragma PROGRAM_FRAGMENT
// Fragment will be forwarded from Shadowmapping.glsl