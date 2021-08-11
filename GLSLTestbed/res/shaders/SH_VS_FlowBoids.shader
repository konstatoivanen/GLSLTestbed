#version 460

#include includes/PKCommon.glsl

PK_DECLARE_BUFFER(float4, pk_Boids);

#pragma PROGRAM_VERTEX

layout(location = 0) in float4 in_POSITION0;

void main()
{
	float4 position = in_POSITION0;

	position.xy += 1.0f;
	position.xy *= 0.5f;

	position.xy -= 0.5f;
	position.xy *= 0.01f;
	position.y *= 2.0f;

	float4 boid = PK_BUFFER_DATA(pk_Boids, gl_InstanceID);

	position.xy += boid.xy;

	position.xy -= 0.5f;
	position.xy *= 2.0f;

	gl_Position = position;
};

#pragma PROGRAM_FRAGMENT

layout(location = 0) out float4 SV_Target0;

void main()
{
	SV_Target0 = float4(1.0f, 0.0f, 0.0f, 1.0f);
};