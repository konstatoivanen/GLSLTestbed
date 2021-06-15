#version 460
#extension GL_ARB_shader_viewport_layer_array : require

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
out float3 gs_POSITION;
out float3 gs_NORMAL;

void main() 
{
	gs_POSITION = ObjectToWorldPos(in_POSITION0);
	gs_NORMAL = ObjectToWorldNormal(in_NORMAL);
	gl_Position = float4(0, 0, 0, 1);
}

#pragma PROGRAM_GEOMETRY
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

const uint2 axis_swizzles[3] = { uint2(0,1), uint2(1,2), uint2(0,2) };

in float3 gs_POSITION[];
in float3 gs_NORMAL[];
out float3 vs_POSITION;
out float3 vs_WOLRDPOSITION;
out float3 vs_NORMAL;

void main() 
{
	float3 v = gs_POSITION[1].xyz - gs_POSITION[0].xyz;
	float3 w = gs_POSITION[2].xyz - gs_POSITION[0].xyz;
	const float3 vnormal = abs(cross(v, w));

	int viewport = 0;
	if (vnormal.z > vnormal.x && vnormal.z > vnormal.y)
	{
		viewport = 0;
	}
	else if (vnormal.x > vnormal.y && vnormal.x > vnormal.z)
	{
		viewport = 1;
	}
	else if (vnormal.y > vnormal.x && vnormal.y > vnormal.z)
	{
		viewport = 2;
	}

	for (uint i = 0; i < 3; ++i) 
	{
		vs_WOLRDPOSITION = gs_POSITION[i];
		vs_NORMAL = gs_NORMAL[i];
		float3 clippos = WorldToClipSpace(gs_POSITION[i]);
		uint2 swizzle = axis_swizzles[viewport];
		gl_Position = float4(clippos[swizzle.x], clippos[swizzle.y], 0, 1);
		gl_ViewportIndex = 0;
		EmitVertex();
	}
	EndPrimitive();
}

#pragma PROGRAM_FRAGMENT
in float3 vs_WOLRDPOSITION;
in float3 vs_NORMAL;

void main() 
{
	if (Greater(abs(WorldToClipSpace(vs_WOLRDPOSITION)), 1.0f.xxx))
	{
		return;
	}

	float3 color = 0.0f.xxx;
	uint cascade = GetShadowCascadeIndex(mul(pk_MATRIX_V, float4(vs_WOLRDPOSITION, 1.0f)).z);

	for (uint i = 0; i < pk_LightCount; ++i)
	{
		PKLight light = GetSurfaceLightDirect(i, vs_WOLRDPOSITION, cascade);
		color += light.color * max(0.0f, dot(light.direction, vs_NORMAL));
	}
	
	StoreSceneGI(vs_WOLRDPOSITION, color);
}