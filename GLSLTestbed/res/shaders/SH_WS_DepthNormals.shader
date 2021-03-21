#version 460

#Blend Off
#ZTest LEqual
#ZWrite On
#Cull Back

#multi_compile _ PK_ENABLE_INSTANCING

#include includes/PKCommon.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float3 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;

out float3 vs_NORMAL;

void main()
{
    gl_Position = ObjectToClipPos(float4(in_POSITION0, 0));
    vs_NORMAL = ObjectToWorldDir(in_NORMAL.xyz);
};

#pragma PROGRAM_FRAGMENT

in float3 vs_NORMAL;

layout(location = 0) out float3 SV_Target0;

void main()
{
    SV_Target0 = normalize(vs_NORMAL) * 0.5f + float3(0.5f);
};