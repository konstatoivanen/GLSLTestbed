#version 460
#extension GL_ARB_shader_viewport_layer_array : require

#Cull Front
#ZTest LEqual
#ZWrite On

#multi_compile _ PK_ENABLE_INSTANCING

#include includes/PKCommon.glsl

#pragma PROGRAM_VERTEX
#include includes/LightingCommon.glsl

const float3 faceSigns[6] =
{
	 float3(-1, -1,  1),
	 float3( 1, -1, -1),

	 float3(1,  1,  1),
	 float3(1, -1, -1),

	 float3( 1, -1,  1),
	 float3(-1, -1, -1),
};

const uint3 swizzles[6] =
{
	uint3(2,1,0),
	uint3(2,1,0),
	
	uint3(0,2,1),
	uint3(0,2,1),
	
	uint3(0,1,2),
	uint3(0,1,2),
};

layout(location = 0) in float3 in_POSITION0;
out noperspective float3 vs_VIEWVECTOR;

void main()
{
	uint lightIndex = 0;
	uint faceIndex = 0;

	#if defined(PK_ENABLE_INSTANCING)
		uint data = PK_INSTANCE_PROPERTIES_ID;
		lightIndex = bitfieldExtract(data, 0, 16);
		faceIndex = bitfieldExtract(data, 24, 8);
		gl_Layer = int(bitfieldExtract(data, 16, 8) * 6 + faceIndex);
	#endif

	float4 light = PK_BUFFER_DATA(pk_Lights, lightIndex).position;
	vs_VIEWVECTOR = ObjectToWorldPos(in_POSITION0) - light.xyz;

	float3 vpos = float3(vs_VIEWVECTOR[swizzles[faceIndex].x], vs_VIEWVECTOR[swizzles[faceIndex].y], vs_VIEWVECTOR[swizzles[faceIndex].z]) * faceSigns[faceIndex];

	gl_Position = float4(vpos.xy, 1.020202f * vpos.z - light.w * 0.020202f, vpos.z);
	gl_ViewportIndex = 0;
};

#pragma PROGRAM_FRAGMENT

in noperspective float3 vs_VIEWVECTOR;
layout(early_fragment_tests) in;
layout(location = 0) out float2 SV_Target0;

void main()
{
	float sqrdist = dot(vs_VIEWVECTOR, vs_VIEWVECTOR);
	SV_Target0 = float2(sqrt(sqrdist), sqrdist);
};