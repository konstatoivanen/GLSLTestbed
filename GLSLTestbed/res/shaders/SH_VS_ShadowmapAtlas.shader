#version 460

#include includes/Lighting.glsl

#pragma PROGRAM_VERTEX
uniform uint pk_ShadowmapIndex;

layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
	float3 st = GetShadowMapTileST(pk_ShadowmapIndex);

	float x = lerp(-1.0f, 1.0f, st.x + in_TEXCOORD0.x * st.z);
	float y = lerp(-1.0f, 1.0f, st.y + in_TEXCOORD0.y * st.z);

	gl_Position = float4(x,y, in_POSITION0.zw);
	vs_TEXCOORD0 = in_TEXCOORD0;
};

#pragma PROGRAM_FRAGMENT

uniform sampler2D _MainTex;

in float2 vs_TEXCOORD0;
layout(location = 0) out float2 SV_Target0;

void main()
{
	SV_Target0 = tex2D(_MainTex, vs_TEXCOORD0).xy;
};