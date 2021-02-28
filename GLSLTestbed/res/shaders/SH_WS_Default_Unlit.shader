#version 460

#multi_compile _ Test1 Test2 Test3
#multi_compile _ Test4
#multi_compile Test5 Test6

#Blend SrcAlpha OneMinusSrcAlpha
#ZTest LEqual
#ZWrite On
#Cull Back

#define PK_ENABLE_INSTANCING
#include PKCommon.glsl

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;
layout(location = 2) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
	gl_Position = ObjectToClipPos(in_POSITION0);
	vs_TEXCOORD0 = in_TEXCOORD0;
};

#pragma PROGRAM_FRAGMENT
in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	float4 color = float4(0);
	float2 uv = vs_TEXCOORD0;

#if defined(Test1)
	uv = float2(1.0f) - uv;
#elif defined(Test2)
	uv.x = 1.0f - uv.x;
#elif defined(Test3)
	uv.y = 1.0f - uv.y;
#endif

#if defined(Test4)
	color.r = 1.0f;
#endif

#if defined(Test5)
	color.rb = uv;
#elif defined(Test6)
	color.bg = uv;
#else
	color.rgb = float3(1.0f);
#endif

	float3 offs = sin(pk_Time.w).xxx * 0.5f + 0.5f;

	SV_Target0 = float4(color.rgb * offs, 1);
};