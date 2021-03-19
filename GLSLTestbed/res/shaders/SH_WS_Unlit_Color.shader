#version 460

#Blend Off
#ZTest LEqual
#ZWrite On
#Cull Back

#include includes/PKCommon.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float3 in_POSITION0;

void main()
{
    gl_Position = ObjectToClipPos(float4(in_POSITION0, 0));
};

#pragma PROGRAM_FRAGMENT
uniform float4 _Color;

layout(location = 0) out float4 SV_Target0;

void main()
{
    SV_Target0 = _Color;
};