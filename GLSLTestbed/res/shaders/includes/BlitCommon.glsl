#ifndef BLIT_COMMON
#define BLIT_COMMON

#include HLSLSupport.glsl

#if defined(USE_VERTEX_PROGRAM_IMG) && defined(SHADER_STAGE_VERTEX)
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
	gl_Position = in_POSITION0;
	vs_TEXCOORD0 = in_TEXCOORD0;
};
#endif

#if defined(INCLUDE_VERTEX_PROGRAM_IMG_VIEWPORT) && defined(SHADER_STAGE_VERTEX)
uniform float4 _BLIT_VIEWPORT;
uniform float4 _BLIT_UVST;

layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out float2 vs_TEXCOORD0;

void main()
{
	float2 xy = in_POSITION0.xy;
	xy *= _BLIT_VIEWPORT.xy;
	xy -= 1.0 - _BLIT_VIEWPORT.xy;
	xy += _BLIT_VIEWPORT.zw * 2.0;

	gl_Position = float4(xy, in_POSITION0.xw);
	vs_TEXCOORD0 = in_TEXCOORD0 * _BLIT_UVST.xy + _BLIT_UVST.zw;
};
#endif

#if defined(INCLUDE_FRAGMENT_PROGRAM_IMG) && defined(SHADER_STAGE_FRAGMENT)
uniform sampler2D _MainTex;

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

void main()
{
	SV_Target0 = tex2D(_MainTex, vs_TEXCOORD0);
};
#endif

#endif