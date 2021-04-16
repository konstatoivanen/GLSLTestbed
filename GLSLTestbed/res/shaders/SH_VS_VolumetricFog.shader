#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_viewport_layer_array : require

#multi_compile FOG_PASS0 FOG_PASS1

#Blend SrcAlpha OneMinusSrcAlpha

#include includes/Lighting.glsl
#include includes/Noise.glsl

uniform highp sampler2D _MainTex;

#define DepthSlices 16

float SampleDepth(float2 uv)
{
	return LinearizeDepth(tex2D(pk_ScreenDepth, uv).r);
}

//----------STRUCTS----------//
struct Varyings
{
	float2 uv;
	float3 origin;
	float3 rayvec;
	float znear;
	float zfar;
};

float GetDensity(float3 wpos)
{
	return unlerp_sat(5.0f, 0.0f, wpos.y);
}

//----------FRAGMENT PROGRAMS----------//
float4 FragmentFogAccumulate(Varyings v)
{
	float scenedepth = SampleDepth(v.uv);
	float zrange = max(0, min(scenedepth, v.zfar) - v.znear);

	float3 viewdir = normalize(v.rayvec);
	float3 wpos = v.origin + viewdir * zrange * NoiseWanghash(v.origin + pk_Time.xyz);

	float3 baseColor = SampleEnv(OctaUV(-viewdir), 1.0f);

	for (uint i = 0; i < pk_LightCount; ++i)
	{
		baseColor += GetVolumeLightColor(i, wpos);
	}

	float density = GetDensity(wpos);

	return float4(baseColor, saturate(zrange * 0.1f) * density);
}

float4 FragmentComposite(Varyings v)
{
	return tex2D(_MainTex, v.uv);
}

#if defined(FOG_PASS0)
#define FUNC_FRAG FragmentFogAccumulate
#elif defined(FOG_PASS1)
#define FUNC_FRAG FragmentComposite
#endif

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out noperspective Varyings vs_Varyings;

void main()
{
	float znear = lerp(pk_ProjectionParams.z, pk_ProjectionParams.y, float(gl_InstanceID) / DepthSlices);
	float zfar = lerp(pk_ProjectionParams.z, pk_ProjectionParams.y, float(gl_InstanceID - 1) / DepthSlices);

	float3 vpos = ClipToViewPos(in_TEXCOORD0.xy, znear);
	float3 vvec = normalize(vpos) * (zfar - znear);

	float3 wpos = mul(pk_MATRIX_I_V, float4(vpos, 1.0f)).xyz;
	float3 wvec = mul(pk_MATRIX_I_V, float4(vvec, 0.0f)).xyz;

	vs_Varyings.uv = in_TEXCOORD0;
	vs_Varyings.origin = wpos;
	vs_Varyings.rayvec = wvec;
	vs_Varyings.znear = znear;
	vs_Varyings.zfar = zfar;
	gl_Position = ViewToClipPos(vpos);
};

#pragma PROGRAM_FRAGMENT
in noperspective Varyings vs_Varyings;
layout(location = 0) out float4 SV_Target0;
void main() { SV_Target0 = FUNC_FRAG(vs_Varyings); };