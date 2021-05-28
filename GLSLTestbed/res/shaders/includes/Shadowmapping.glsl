#pragma once
#include Lighting.glsl

layout(binding = 0) uniform highp samplerCubeArray _ShadowmapBatchCube;
layout(binding = 1) uniform highp sampler2DArray _ShadowmapBatch0;
layout(binding = 2) uniform highp sampler2DArray _ShadowmapBatch1;

#define SAMPLE_COUNT 4u
#define SAMPLE_COUNT_INV 0.25f
#define SHADOW_NEAR_BIAS 0.1f
#define SHADOW_MAP_BATCH_SIZE 4
// @TODO Parameterize these later
#define GET_SHADOW_BLUR_AMOUNT_POINT pow5(0.1f)
#define GET_SHADOW_BLUR_AMOUNT_SPOT 13.0f / 1024.0f
#define GET_SHADOW_BLUR_AMOUNT_DIRECTIONAL 6.5f / 1024.0f

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
    float3(-4.371139e-08f,  1.0f,           0.5f),
    float3(-1.0f,          -8.742278e-08f,  0.25f),
    float3(1.192488e-08f,  -1.0f,           0.75f),
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
        #if defined(SHADOW_BLUR_PASS1)
            sampleLayer += gl_BaseInstance + SHADOW_MAP_BATCH_SIZE;
        #endif
        gl_Layer = gl_BaseInstance + gl_InstanceID;                                                     
        gl_ViewportIndex = 1;                                                                           
    #endif
}

const int2 sample_offsets[4] = { int2(1,0), int2(0,1), int2(-1,0), int2(0,-1)};
const int2 sample_offsets_h0[4] = { int2(-2, 0), int2(-1,0), int2(1,0), int2(2,0)};
const int2 sample_offsets_h1[4] = { int2(-3, 0), int2(0, 0), int2(3,0), int2(0,0)};

const int2 sample_offsets_v0[4] = { int2( 0,-2), int2(0,-1), int2(0,1), int2(0,2)};
const int2 sample_offsets_v1[4] = { int2( 0,-3), int2(0, 0), int2(0,3), int2(0,0)};

#if defined(SHADOW_BLUR_PASS0)
    float2 SAMPLE_SRC(float3 uvw)
    {
        float4 valueR0 = textureGatherOffsets(_ShadowmapBatch0, uvw, sample_offsets_h0, 0);
        float4 valueG0 = textureGatherOffsets(_ShadowmapBatch0, uvw, sample_offsets_h0, 1);
        float3 valueR1 = textureGatherOffsets(_ShadowmapBatch0, uvw, sample_offsets_h1, 0).xyz;
        float3 valueG1 = textureGatherOffsets(_ShadowmapBatch0, uvw, sample_offsets_h1, 1).xyz;

        return float2
        (
            dot(valueR0, float4(0.0205, 0.0855, 0.0855, 0.0205)) + dot(valueR1, float3(0.232, 0.324, 0.232)),
            dot(valueG0, float4(0.0205, 0.0855, 0.0855, 0.0205)) + dot(valueG1, float3(0.232, 0.324, 0.232))
        );
    }

    // Unfortunately gathering is not supported for cube maps.
    #define SAMPLE_SRC_OCT(H, layer) tex2D(_ShadowmapBatchCube, float4(H, layer)).rg
#else
    float2 SAMPLE_SRC(float3 uvw)
    {
        float4 valueR0 = textureGatherOffsets(_ShadowmapBatch1, uvw, sample_offsets_v0, 0);
        float4 valueG0 = textureGatherOffsets(_ShadowmapBatch1, uvw, sample_offsets_v0, 1);
        float3 valueR1 = textureGatherOffsets(_ShadowmapBatch1, uvw, sample_offsets_v1, 0).xyz;
        float3 valueG1 = textureGatherOffsets(_ShadowmapBatch1, uvw, sample_offsets_v1, 1).xyz;

        return float2
        (
            dot(valueR0, float4(0.0205, 0.0855, 0.0855, 0.0205)) + dot(valueR1, float3(0.232, 0.324, 0.232)),
            dot(valueG0, float4(0.0205, 0.0855, 0.0855, 0.0205)) + dot(valueG1, float3(0.232, 0.324, 0.232))
        );
    }

    float2 SAMPLE_SRC_OCT(float3 H, float layer) 
    {
        float3 uvw = float3(OctaUV(H), layer);
        return float2(dot(textureGatherOffsets(_ShadowmapBatch1, uvw, sample_offsets, 0), 0.25f.xxxx), dot(textureGatherOffsets(_ShadowmapBatch1, uvw, sample_offsets, 1), 0.25f.xxxx));
    }
#endif

#if defined(SHADER_STAGE_FRAGMENT) && defined(DRAW_SHADOW_MAP_FRAGMENT)
in noperspective float vs_DEPTH;
layout(early_fragment_tests) in;
layout(location = 0) out float2 SV_Target0;

void main() { SV_Target0 = float2(vs_DEPTH, vs_DEPTH * vs_DEPTH); };
#endif