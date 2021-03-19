#version 460

#multi_compile BLOOM_PASS0 BLOOM_PASS1 BLOOM_PASS2 BLOOM_PASS3

#include includes/HLSLSupport.glsl

uniform highp sampler2D _MainTex;
uniform highp sampler2D _Bloom_Textures[6];
uniform highp sampler2D _Bloom_DirtTexture;

uniform float _Bloom_BlurSize;
uniform float _Bloom_Intensity;
uniform float _Bloom_LensDirtIntensity;

uniform float _Tonemap_Exposure = 1.0f;

#define _Maintex_TexelSize (float2(1.0f,1.0f) / textureSize(_MainTex, 0).xy)

const float curve[7] = { 0.0205, 0.0855, 0.232, 0.324, 0.232, 0.0855, 0.0205 };

//----------STRUCTS----------//
struct v2f_simple
{
	float2 uv;
};

struct v2f_withBlurCoords8
{
	float2 uv0; 
	float2 uv1;
	float2 uv2;
	float2 uv3;
	float2 uv4;
	float2 uv5;
	float2 uv6;
};

struct v2f_tap
{
	float2 uv20;
	float2 uv21;
	float2 uv22;
	float2 uv23;
};

float3 tonemapHejlDawson(half3 color, float exposure)
{
	const half a = 6.2;
	const half b = 0.5;
	const half c = 1.7;
	const half d = 0.06;

	color *= exposure;
	color = max(float3(0.0), color - float3(0.004));
	color = (color * (a * color + b)) / (color * (a * color + c) + d);
	return color * color;
}

float3 saturation(float3 color, float amount) 
{
	float grayscale = dot(color, float3(0.3, 0.59, 0.11));
	return lerp_true(grayscale.xxx, color, 0.8f);
}

//----------VERTEX PROGRAMS----------//
v2f_simple vertBloom(float2 texcoord)
{
	v2f_simple o;
	o.uv = texcoord.xy;
	return o;
}

v2f_tap vert4Tap(float2 texcoord)
{
	v2f_tap o;
	o.uv20 = texcoord + _Maintex_TexelSize.xy;
	o.uv21 = texcoord - _Maintex_TexelSize.xy;
	o.uv22 = texcoord + _Maintex_TexelSize.xy * float2( 1.0f, -1.0f);
	o.uv23 = texcoord + _Maintex_TexelSize.xy * float2(-1.0f, 1.0f);
	return o;
}

v2f_withBlurCoords8 vertBlurVertical(float2 texcoord)
{
	v2f_withBlurCoords8 o;
	float2 offs = _Maintex_TexelSize.xy * half2(0.0, 1.0) * _Bloom_BlurSize;
	float2 coords = texcoord - offs * 3.0f;
	o.uv0 = coords + offs * 0.0f;
	o.uv1 = coords + offs * 1.0f;
	o.uv2 = coords + offs * 2.0f;
	o.uv3 = coords + offs * 3.0f;
	o.uv4 = coords + offs * 4.0f;
	o.uv5 = coords + offs * 5.0f;
	o.uv6 = coords + offs * 6.0f;
	return o;
}

v2f_withBlurCoords8 vertBlurHorizontal(float2 texcoord)
{
	v2f_withBlurCoords8 o;

	float2 offs = _Maintex_TexelSize.xy * float2(1.0f, 0.0f) * _Bloom_BlurSize;

	float2 coords = texcoord - offs * 3.0f;
	o.uv0 = coords + offs * 0.0f;
	o.uv1 = coords + offs * 1.0f;
	o.uv2 = coords + offs * 2.0f;
	o.uv3 = coords + offs * 3.0f;
	o.uv4 = coords + offs * 4.0f;
	o.uv5 = coords + offs * 5.0f;
	o.uv6 = coords + offs * 6.0f;
	return o;
}

//----------FRAGMENT PROGRAMS----------//
float4 fragBloom(v2f_simple i)
{
	float3 color = tex2D(_MainTex, i.uv).rgb;
	
	float3 lens = tex2D(_Bloom_DirtTexture, i.uv).rgb;
	float3 b0 = tex2D(_Bloom_Textures[0], i.uv).rgb;
	float3 b1 = tex2D(_Bloom_Textures[1], i.uv).rgb;
	float3 b2 = tex2D(_Bloom_Textures[2], i.uv).rgb;
	float3 b3 = tex2D(_Bloom_Textures[3], i.uv).rgb;
	float3 b4 = tex2D(_Bloom_Textures[4], i.uv).rgb;
	float3 b5 = tex2D(_Bloom_Textures[5], i.uv).rgb;

	float3 bloom = b0 * 0.5f
				 + b1 * 0.8f * 0.75f
				 + b2 * 0.6f
				 + b3 * 0.45f
				 + b4 * 0.35f
				 + b5 * 0.23f;

	bloom /= 2.2f;

	float3 lensBloom = b0 * 1.0f + b1 * 0.8f + b2 * 0.6f + b3 * 0.45f + b4 * 0.35f + b5 * 0.23f;
	lensBloom /= 3.2f;

	color = lerp(color, bloom, float3(_Bloom_Intensity));
	color.r = lerp(color.r, lensBloom.r, (saturate(lens.r * _Bloom_LensDirtIntensity)));
	color.g = lerp(color.g, lensBloom.g, (saturate(lens.g * _Bloom_LensDirtIntensity)));
	color.b = lerp(color.b, lensBloom.b, (saturate(lens.b * _Bloom_LensDirtIntensity)));

	color = saturation(color, 0.8f);
	color = tonemapHejlDawson(color, _Tonemap_Exposure);

	return float4(color, 1.0f);
}

float4 fragDownsample(v2f_tap i)
{
	float4 color = tex2D(_MainTex, i.uv20);
	color += tex2D(_MainTex, i.uv21);
	color += tex2D(_MainTex, i.uv22);
	color += tex2D(_MainTex, i.uv23);
	return max(color / 4.0f, 0);
}

float4 fragBlur8(v2f_withBlurCoords8 i)
{
	float3 color = tex2D(_MainTex, i.uv0).rgb * curve[0];
	color += tex2D(_MainTex, i.uv1).rgb * curve[1];
	color += tex2D(_MainTex, i.uv2).rgb * curve[2];
	color += tex2D(_MainTex, i.uv3).rgb * curve[3];
	color += tex2D(_MainTex, i.uv4).rgb * curve[4];
	color += tex2D(_MainTex, i.uv5).rgb * curve[5];
	color += tex2D(_MainTex, i.uv6).rgb * curve[6];
	return float4(color, 1.0f);
}

#if defined(BLOOM_PASS0)
	#define VS_VARYINGS v2f_simple
	#define FUNC_VERTEX vertBloom
	#define FUNC_FRAG fragBloom
#elif defined(BLOOM_PASS1)
	#define VS_VARYINGS v2f_tap
	#define FUNC_VERTEX vert4Tap
	#define FUNC_FRAG fragDownsample
#elif defined(BLOOM_PASS2)
	#define VS_VARYINGS v2f_withBlurCoords8
	#define FUNC_VERTEX vertBlurVertical
	#define FUNC_FRAG fragBlur8
#elif defined(BLOOM_PASS3)
	#define VS_VARYINGS v2f_withBlurCoords8
	#define FUNC_VERTEX vertBlurHorizontal
	#define FUNC_FRAG fragBlur8
#endif

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out VS_VARYINGS vs_Varyings;

void main()
{
	gl_Position = in_POSITION0;
	vs_Varyings = FUNC_VERTEX(in_TEXCOORD0);
};

#pragma PROGRAM_FRAGMENT
in VS_VARYINGS vs_Varyings;
layout(location = 0) out float4 SV_Target0;
void main() { SV_Target0 = FUNC_FRAG(vs_Varyings); };