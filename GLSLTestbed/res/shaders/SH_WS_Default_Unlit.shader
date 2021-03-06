#version 460

#Blend Off
#ZTest LEqual
#ZWrite On
#Cull Back

#define PK_ENABLE_INSTANCING
#include Lighting.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float3 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;
layout(location = 2) in float2 in_TEXCOORD0;

out float2 vs_TEXCOORD0;
out float3 vs_NORMAL;
out float3 vs_VIEWDIRECTION;

void main()
{
	gl_Position = ObjectToClipPos(float4(in_POSITION0, 0));
	vs_TEXCOORD0 = in_TEXCOORD0;
    vs_VIEWDIRECTION = (pk_WorldSpaceCameraPos.xyz - ObjectToWorldPos(in_POSITION0.xyz));
    vs_NORMAL = ObjectToWorldDir(in_NORMAL.xyz);
};

#pragma PROGRAM_FRAGMENT
in float2 vs_TEXCOORD0;
in float3 vs_NORMAL;
in float3 vs_VIEWDIRECTION;
layout(location = 0) out float4 SV_Target0;

void main()
{
    float3 viewdir = normalize(vs_VIEWDIRECTION);

    SurfaceData surf; 
    surf.albedo = float3(1,1,1);
    surf.normal = vs_NORMAL;
    surf.emission = float3(0,0,0);
    surf.metallic = 0.1f;
    surf.roughness = 0.25f;
    surf.occlusion = 0.0f;
    surf.alpha = 1.0f;
    
    SV_Target0 = PhysicallyBasedShading(surf, viewdir);
};