#pragma once
#include Lighting.glsl

layout(binding = 0) uniform highp samplerCubeArray _ShadowmapBatchCube;
layout(binding = 1) uniform highp sampler2DArray _ShadowmapBatch0;
layout(binding = 2) uniform highp sampler2DArray _ShadowmapBatch1;

#define SAMPLE_COUNT 16u
// @TODO Parameterize these later
#define GET_SHADOW_BLUR_AMOUNT_POINT pow5(0.125f)
#define GET_SHADOW_BLUR_AMOUNT_SPOT 13.0f / 512.0f
#define GET_SHADOW_BLUR_AMOUNT_DIRECTIONAL 6.5f / 512.0f

const float3 faceSigns[6] =
{
	 float3(-1, -1,  1),
	 float3( 1, -1, -1),

	 float3(1,  1,  1),
	 float3(1, -1, -1),

	 float3( 1, -1,  1),
	 float3(-1, -1, -1),
};

const uint3 swizzles[6] =
{
	uint3(2,1,0),
	uint3(2,1,0),
	
	uint3(0,2,1),
	uint3(0,2,1),
	
	uint3(0,1,2),
	uint3(0,1,2),
};

const float3 SAMPLES_HAMMERSLEY_3D[SAMPLE_COUNT] =
{
    float3(1.0f,            0.0f,           0.0f),
    float3(0.9238795f,      0.3826835f,     0.5f),
    float3(0.7071068f,      0.7071068f,     0.25f),
    float3(0.3826834f,      0.9238795f,     0.75f),
    float3(-4.371139E-08f,  1.0f,           0.125f),
    float3(-0.3826835f,     0.9238795f,     0.625f),
    float3(-0.7071068f,     0.7071068f,     0.375f),
    float3(-0.9238796f,     0.3826833f,     0.875f),
    float3(-1.0f,           -8.742278E-08f, 0.0625f),
    float3(-0.9238795f,     -0.3826834f,    0.5625f),
    float3(-0.7071066f,     -0.7071069f,    0.3125f),
    float3(-0.3826831f,     -0.9238797f,    0.8125f),
    float3(1.192488E-08f,   -1.0f,          0.1875f),
    float3(0.3826836f,      -0.9238794f,    0.6875f),
    float3(0.707107f,       -0.7071065f,    0.4375f),
    float3(0.9238796f,      -0.3826834f,    0.9375f),
};

const float2 SAMPLES_HAMMERSLEY_2D[SAMPLE_COUNT] =
{
    float2(-0.5,-0.5),
    float2(-0.4333333,0),
    float2(-0.3666667,-0.25),
    float2(-0.3,0.25),
    float2(-0.2333333,-0.375),
    float2(-0.1666667,0.125),
    float2(-0.09999999,-0.125),
    float2(-0.03333333,0.375),
    float2(0.03333336,-0.4375),
    float2(0.1,0.0625),
    float2(0.1666667,-0.1875),
    float2(0.2333333,0.3125),
    float2(0.3,-0.3125),
    float2(0.3666667,0.1875),
    float2(0.4333333,-0.0625),
    float2(0.5,0.4375),
};

struct DrawIndices
{
	uint lightIndex; 
	uint batchIndex;
	uint faceIndex;
};

float3 DistributeHammersley3D(float3 Xi, float blur)
{
    float theta = (1.0f - Xi.z) / (1.0f + (blur - 1.0f) * Xi.z);
    float2 sincos = sqrt(float2(1.0f - theta, theta));
    return normalize(float3(Xi.xy * sincos.xx, sincos.y));
}

float2 DistributeHammersley2D(float2 Xi, float blur)
{
    return Xi * blur;
}

DrawIndices ParseDrawIndices(uint data)
{
	return DrawIndices(bitfieldExtract(data, 0, 16), bitfieldExtract(data, 16, 8), bitfieldExtract(data, 24, 8));
}

float4 GetCubeClipPos(float3 viewvec, float radius, uint faceIndex)
{
	float3 vpos = float3(viewvec[swizzles[faceIndex].x], viewvec[swizzles[faceIndex].y], viewvec[swizzles[faceIndex].z]) * faceSigns[faceIndex];
	return float4(vpos.xy, 1.020202f * vpos.z - radius * 0.020202f, vpos.z);
}

void SHADOW_SET_VERTEX_STATE_ATTRIBUTES(float4 position, float2 baseuv, out uint sampleLayer)
{
    #if defined(SHADER_STAGE_VERTEX)
        gl_Position = position;        
        sampleLayer = gl_InstanceID;
        gl_Layer = gl_BaseInstance + gl_InstanceID;                                                     
        gl_ViewportIndex = 1;                                                                           
    #endif
}

#if defined(SHADOW_BLUR_PASS0)
    #define SAMPLE_SRC(UV, layer) tex2D(_ShadowmapBatch0, float3(UV, layer)).rg
    #define SAMPLE_SRC_OCT(H, layer) tex2D(_ShadowmapBatchCube, float4(H, layer)).rg
#else
    #define SAMPLE_SRC(UV, layer) tex2D(_ShadowmapBatch1, float3(UV, layer)).rg;
    #define SAMPLE_SRC_OCT(H, layer) tex2D(_ShadowmapBatch1, float3(OctaUV(H), layer)).rg;
#endif

#if defined(SHADER_STAGE_FRAGMENT) && defined(DRAW_SHADOW_MAP_FRAGMENT)
in noperspective float3 vs_VIEWVECTOR;
layout(early_fragment_tests) in;
layout(location = 0) out float2 SV_Target0;

void main()
{
	float sqrdist = dot(vs_VIEWVECTOR, vs_VIEWVECTOR);
	SV_Target0 = float2(sqrt(sqrdist), sqrdist);
};
#endif