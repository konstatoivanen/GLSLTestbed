#version 460

#Cull Front
#ZTest LEqual
#ZWrite On

#multi_compile _ PK_ENABLE_INSTANCING

#include includes/PKCommon.glsl
#include includes/LightingCommon.glsl
uniform uint pk_ShadowmapLightIndex;

#pragma PROGRAM_VERTEX

layout(location = 0) in float4 in_POSITION0;

void main()
{
	gl_Position = float4(ObjectToWorldPos(in_POSITION0.xyz), 1.0f);
};

#pragma PROGRAM_GEOMETRY

const uint3 axisswizzles[3] =
{
	uint3(2,1,0),
	uint3(0,2,1),
	uint3(0,1,2),
};

const float3 facesigns[6] = 
{
	// X
    float3(-1, -1, 1),
    float3( 1, -1, -1),
	// Y
    float3( 1,  1,  1),
	float3( 1, -1, -1),
	// Z
	float3( 1, -1,  1),
    float3(-1, -1, -1),
};

uniform float4x4 pk_ShadowmapMatrix;

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

out noperspective float3 vs_POSITION;

void main()
{
	float4 lposition = PK_BUFFER_DATA(pk_Lights, pk_ShadowmapLightIndex).position;
	float3 center = lposition.xyz;
	float  zfar = lposition.w;

	uint3 swizzle = axisswizzles[gl_InvocationID / 2];
	float3 fsign = facesigns[gl_InvocationID];

	gl_Layer = gl_InvocationID;

	float3 outpositions[3];
	float4 viewpositions[3];
	float2 clippositions[3];

	outpositions[0] = gl_in[0].gl_Position.xyz;
	outpositions[1] = gl_in[1].gl_Position.xyz;
	outpositions[2] = gl_in[2].gl_Position.xyz;

	viewpositions[0] = float4(outpositions[0] - center, 1.0f);
	viewpositions[1] = float4(outpositions[1] - center, 1.0f);
	viewpositions[2] = float4(outpositions[2] - center, 1.0f);

	viewpositions[0].xyz = float3(viewpositions[0][swizzle.x], viewpositions[0][swizzle.y], viewpositions[0][swizzle.z]) * fsign;
	viewpositions[1].xyz = float3(viewpositions[1][swizzle.x], viewpositions[1][swizzle.y], viewpositions[1][swizzle.z]) * fsign;
	viewpositions[2].xyz = float3(viewpositions[2][swizzle.x], viewpositions[2][swizzle.y], viewpositions[2][swizzle.z]) * fsign;

	viewpositions[0] = mul(pk_ShadowmapMatrix, viewpositions[0]);
	viewpositions[1] = mul(pk_ShadowmapMatrix, viewpositions[1]);
	viewpositions[2] = mul(pk_ShadowmapMatrix, viewpositions[2]);

	// ClipToScreenPos
	clippositions[0] = viewpositions[0].xy / viewpositions[0].w;
	clippositions[1] = viewpositions[1].xy / viewpositions[1].w;
	clippositions[2] = viewpositions[2].xy / viewpositions[2].w;

	float2 minp = min(min(clippositions[0], clippositions[1]), clippositions[2]);
	float2 maxp = max(max(clippositions[0], clippositions[1]), clippositions[2]);

	if (Less(maxp, float2(-1.0f)) || Greater(minp, float2(1.0f)))
	{
		return;
	}

	vs_POSITION = outpositions[0];
	gl_Position = viewpositions[0];
	EmitVertex();

	vs_POSITION = outpositions[1];
	gl_Position = viewpositions[1];
	EmitVertex();

	vs_POSITION = outpositions[2];
	gl_Position = viewpositions[2];
	EmitVertex();

	EndPrimitive();
}

#pragma PROGRAM_FRAGMENT

in float3 vs_POSITION;
layout(early_fragment_tests) in;
layout(location = 0) out float2 SV_Target0;

void main()
{
	PKRawLight light = PK_BUFFER_DATA(pk_Lights, pk_ShadowmapLightIndex);

	float3 lvector = vs_POSITION - light.position.xyz;
	float dist = length(lvector);
	float sqrdist = dist * dist;

	SV_Target0 = float2(dist, sqrdist);
};