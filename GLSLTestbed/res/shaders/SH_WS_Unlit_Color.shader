#version 460

#Blend Off
#ZTest Equal
#ZWrite Off
#Cull Back

#multi_compile _ PK_ENABLE_INSTANCING

#include includes/PKCommon.glsl

PK_BEGIN_INSTANCED_PROPERTIES
    PK_INSTANCED_PROPERTY float4 _Color;
PK_END_INSTANCED_PROPERTIES

#pragma PROGRAM_VERTEX
layout(location = 0) in float3 in_POSITION0;
PK_VARYING_INSTANCE_ID

void main()
{
    PK_SETUP_INSTANCE_ID();
    gl_Position = ObjectToClipPos(float4(in_POSITION0, 0));
};

#pragma PROGRAM_FRAGMENT
PK_VARYING_INSTANCE_ID

layout(location = 0) out float4 SV_Target0;

void main()
{
    PK_SETUP_INSTANCE_ID();
    SV_Target0 = PK_ACCESS_INSTANCED_PROP(_Color);
};