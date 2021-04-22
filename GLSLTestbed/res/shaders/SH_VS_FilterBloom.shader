#version 460
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_viewport_layer_array : require

#multi_compile BLOOM_PASS0 BLOOM_PASS1 BLOOM_PASS2

#include includes/PKCommon.glsl
#include includes/HLSLSupport.glsl

struct PassData
{
	highp sampler2DArray source;
	float2 offset;
	uint2 readwrite;
};

PK_DECLARE_RESTRICTED_READONLY_BUFFER(PassData, _BloomPassParams);

const float curve[7] = { 0.0205, 0.0855, 0.232, 0.324, 0.232, 0.0855, 0.0205 };

//----------STRUCTS----------//
struct VaryingsUV8
{
	float3 uv0; 
	float3 uv1;
	float3 uv2;
	float3 uv3;
	float3 uv4;
	float3 uv5;
	float3 uv6;
};

struct VaryingsUV4
{
	float3 uv20;
	float3 uv21;
	float3 uv22;
	float3 uv23;
};

float3 TonemapHejlDawson(half3 color, float exposure)
{
	const half a = 6.2;
	const half b = 0.5;
	const half c = 1.7;
	const half d = 0.06;

	color *= exposure;
	color = max(float3(0.0), color - 0.004);
	color = (color * (a * color + b)) / (color * (a * color + c) + d);
	return color * color;
}

float3 Saturation(float3 color, float amount) 
{
	float grayscale = dot(color, float3(0.3, 0.59, 0.11));
	return lerp_true(grayscale.xxx, color, 0.8f);
}

float3 LinearToGamma(float3 color)
{
	//Source: http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
	float3 S1 = sqrt(color);
	float3 S2 = sqrt(S1);
	float3 S3 = sqrt(S2);
	return 0.662002687 * S1 + 0.684122060 * S2 - 0.323583601 * S3 - 0.0225411470 * color;
}

//----------VERTEX PROGRAMS----------//
float2 VertexSimple(float3 texcoord, sampler2DArray source, float2 offset)
{
	return texcoord.xy;
}

VaryingsUV4 VertexBlurUV4(float3 texcoord, sampler2DArray source, float2 offset)
{
	VaryingsUV4 o;
	o.uv20 = float3(texcoord.xy + 0.5f / textureSize(source, 0).xy, texcoord.z);
	o.uv21 = float3(texcoord.xy - 0.5f / textureSize(source, 0).xy, texcoord.z);
	o.uv22 = float3(texcoord.xy + float2( 0.5f, -0.5f) / textureSize(source, 0).xy, texcoord.z);
	o.uv23 = float3(texcoord.xy + float2(-0.5f,  0.5f) / textureSize(source, 0).xy, texcoord.z);
	return o;
}

VaryingsUV8 VertexBlurUV8(float3 texcoord, sampler2DArray source, float2 offset)
{
	VaryingsUV8 o;
	float3 offs = float3(offset / textureSize(source, 0).xy, 0);
	float3 coords = texcoord - offs * 3.0f;
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
float4 FragmentComposite(float2 uv, sampler2DArray source)
{
	float3 uvw = float3(uv, 0);
	PassData pass30 = PK_BUFFER_DATA(_BloomPassParams, 30);
	PassData pass31 = PK_BUFFER_DATA(_BloomPassParams, 31);

	float bloomIntensity = pass30.offset.x;
	float bloomDirtIntensity = pass30.offset.y;
	float toneMappingExposure = pass31.offset.x;
	float saturation = pass31.offset.y;

	float3 color = tex2D(pass30.source, uvw).rgb;
	float3 lensdirt = tex2D(pass31.source, uvw).rgb;
	float3 b0 = tex2D(PK_BUFFER_DATA(_BloomPassParams, 1).source, uvw).rgb;
	float3 b1 = tex2D(PK_BUFFER_DATA(_BloomPassParams, 6).source, uvw).rgb;
	float3 b2 = tex2D(PK_BUFFER_DATA(_BloomPassParams, 11).source, uvw).rgb;
	float3 b3 = tex2D(PK_BUFFER_DATA(_BloomPassParams, 16).source, uvw).rgb;
	float3 b4 = tex2D(PK_BUFFER_DATA(_BloomPassParams, 21).source, uvw).rgb;
	float3 b5 = tex2D(PK_BUFFER_DATA(_BloomPassParams, 26).source, uvw).rgb;

	float3 bloom = b0 * 0.5f + b1 * 0.6f + b2 * 0.6f + b3 * 0.45f + b4 * 0.35f + b5 * 0.23f;
	float3 bloomLens = b0 * 1.0f + b1 * 0.8f + b2 * 0.6f + b3 * 0.45f + b4 * 0.35f + b5 * 0.23f;

	bloom /= 2.2f;
	bloomLens /= 3.2f;

	color = lerp(color, bloom, float3(bloomIntensity));
	color.rgb = lerp(color.rgb, bloomLens.rgb, saturate(lensdirt.rgb * bloomDirtIntensity));
	
	color = Saturation(color, 0.8f);
	color = TonemapHejlDawson(color, toneMappingExposure);
	color = LinearToGamma(color);

	return float4(color, 1.0f);
}

float4 FragmentBlurUV4(VaryingsUV4 i, sampler2DArray source)
{
	float4 color = tex2D(source, i.uv20);
	color += tex2D(source, i.uv21);
	color += tex2D(source, i.uv22);
	color += tex2D(source, i.uv23);
	return max(color / 4.0f, 0.0f);
}

float4 FragmentBlurUV8(VaryingsUV8 i, sampler2DArray source)
{
	float3 color = tex2D(source, i.uv0).rgb * curve[0];
	color += tex2D(source, i.uv1).rgb * curve[1];
	color += tex2D(source, i.uv2).rgb * curve[2];
	color += tex2D(source, i.uv3).rgb * curve[3];
	color += tex2D(source, i.uv4).rgb * curve[4];
	color += tex2D(source, i.uv5).rgb * curve[5];
	color += tex2D(source, i.uv6).rgb * curve[6];
	return float4(color, 1.0f);
}

#if defined(BLOOM_PASS0)
	#define VS_VARYINGS float2
	#define FUNC_VERTEX VertexSimple
	#define FUNC_FRAG FragmentComposite
#elif defined(BLOOM_PASS1)
	#define VS_VARYINGS VaryingsUV4
	#define FUNC_VERTEX VertexBlurUV4
	#define FUNC_FRAG FragmentBlurUV4
#elif defined(BLOOM_PASS2)
	#define VS_VARYINGS VaryingsUV8
	#define FUNC_VERTEX VertexBlurUV8
	#define FUNC_FRAG FragmentBlurUV8
#endif

#pragma PROGRAM_VERTEX
layout(location = 0) in float4 in_POSITION0;
layout(location = 1) in float2 in_TEXCOORD0;
out flat sampler2DArray vs_MainTex;
out VS_VARYINGS vs_Varyings;

void main()
{
	PassData passdata = PK_BUFFER_DATA(_BloomPassParams, gl_BaseInstance);
	vs_MainTex = passdata.source;
	vs_Varyings = FUNC_VERTEX(float3(in_TEXCOORD0, passdata.readwrite.x), passdata.source, passdata.offset);
	gl_Layer = int(passdata.readwrite.y);
	gl_ViewportIndex = gl_BaseInstance / 5;
	gl_Position = in_POSITION0;
};

#pragma PROGRAM_FRAGMENT
in flat sampler2DArray vs_MainTex;
in VS_VARYINGS vs_Varyings;
layout(location = 0) out float4 SV_Target0;
void main() { SV_Target0 = FUNC_FRAG(vs_Varyings, vs_MainTex); };