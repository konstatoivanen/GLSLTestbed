#version 460

#Blend Off
#ZTest LEqual
#ZWrite On
#Cull Off

#include includes/PKCommon.glsl

#pragma PROGRAM_VERTEX
struct Vertex
{
    float4 position;
    float4 color;
};


PK_DECLARE_BUFFER(Vertex, pk_GizmoVertices);

out float4 vs_COLOR;

void main()
{
    Vertex v = PK_BUFFER_DATA(pk_GizmoVertices, PK_VERTEX_ID);
    gl_Position = ObjectToClipPos(v.position);
    vs_COLOR = v.color;
};

#pragma PROGRAM_FRAGMENT

in float4 vs_COLOR;
layout(location = 0) out float4 SV_Target0;

void main()
{
    SV_Target0 = vs_COLOR;
};