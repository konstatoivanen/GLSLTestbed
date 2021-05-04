#pragma once
#ifndef PK_COMMON
#define PK_COMMON

#extension GL_ARB_bindless_texture : require

#include HLSLSupport.glsl
#include Instancing.glsl

#define PK_PI            3.14159265359f
#define PK_TWO_PI        6.28318530718f
#define PK_FOUR_PI       12.56637061436f
#define PK_INV_PI        0.31830988618f
#define PK_INV_TWO_PI    0.15915494309f
#define PK_INV_FOUR_PI   0.07957747155f
#define PK_HALF_PI       1.57079632679f
#define PK_INV_HALF_PI   0.636619772367f

#define pk_Grey float4(0.214041144, 0.214041144, 0.214041144, 0.5)
// standard dielectric reflectivity coef at incident angle (= 4%)
#define pk_DielectricSpecular float4(0.04, 0.04, 0.04, 1.0 - 0.04) 
#define pk_Luminance float4(0.2125, 0.7154, 0.0721, 1.0f) //float4(0.0396819152, 0.458021790, 0.00609653955, 1.0) //

PK_DECLARE_CBUFFER(pk_PerFrameConstants)
{
    // Time since level load (t/20, t, t*2, t*3), use to animate things inside the shaders.
    float4 pk_Time;
    // Sine of time: (t/8, t/4, t/2, t).
    float4 pk_SinTime;
    // Cosine of time: (t/8, t/4, t/2, t).
    float4 pk_CosTime;
    // Delta time: (dt, 1/dt, smoothDt, 1/smoothDt).
    float4 pk_DeltaTime;
    
    // World space position of the camera.
    float4 pk_WorldSpaceCameraPos;
    // x = n, y = f, z = f - n, w = 1.0f / f.
    float4 pk_ProjectionParams;
    // x = 1.0f / log2(f / n), y = -log2(n) / log2(f / n), z = f / n, w = 1.0f / n.
    float4 pk_ExpProjectionParams;
    // x is the width of the camera’s target texture in pixels, y is the height of the camera’s target texture in pixels, z is 1.0 + 1.0/width and w is 1.0 + 1.0/height.
    float4 pk_ScreenParams;
    // view space z axis splits for directional light shadow cascades
    float4 pk_ShadowCascadeZSplits;

    // Current view matrix.
    float4x4 pk_MATRIX_V;
    // Current inverse view matrix.
    float4x4 pk_MATRIX_I_V;
    // Current projection matrix.
    float4x4 pk_MATRIX_P;
    // Current inverse projection matrix.
    float4x4 pk_MATRIX_I_P;
    // Current view * projection matrix.
    float4x4 pk_MATRIX_VP;
    // Current inverse view * projection matrix.
    float4x4 pk_MATRIX_I_VP;

    // Scene reflections
    sampler2D pk_SceneOEM_HDR;
    // Scene normals
    highp sampler2D pk_ScreenNormals;
    // Scene depth
    highp sampler2D pk_ScreenDepth;
    // Scene shadowmap atlas
    highp sampler2DArray pk_ShadowmapArray;
    // Scene ambient occlusion
    sampler2D pk_ScreenOcclusion;
    // Array of light cookies
    sampler2DArray pk_LightCookies;
    // Global blue noise texture
    sampler2D pk_Bluenoise256;

    // Scene reflections exposure
    float pk_SceneOEM_Exposure;
};

#if defined(PK_ENABLE_INSTANCING)
    PK_DECLARE_READONLY_BUFFER(float4x4, pk_InstancingMatrices);
    #define pk_MATRIX_M PK_BUFFER_DATA(pk_InstancingMatrices, PK_INSTANCE_OFFSET_ID)
    #define pk_MATRIX_I_M inverse(PK_BUFFER_DATA(pk_InstancingMatrices, PK_INSTANCE_OFFSET_ID))
#else
    // Current model matrix.
    uniform float4x4 pk_MATRIX_M;
    // Current inverse model matrix.
    uniform float4x4 pk_MATRIX_I_M;
#endif

float LinearizeDepth(float z) 
{ 
    return 1.0f / (pk_MATRIX_I_P[2][3] * (z * 2.0f - 1.0f) + pk_MATRIX_I_P[3][3]);
} 

float3 GlobalNoiseBlue(uint2 coord)
{
	return texelFetch(pk_Bluenoise256, int2(coord.x % 256, coord.y % 256), 0).xyz;
}

float4 WorldToClipPos( in float3 pos) { return mul(pk_MATRIX_VP, float4(pos, 1.0)); }

float4 ViewToClipPos( in float3 pos) { return mul(pk_MATRIX_P, float4(pos, 1.0)); }

float3 WorldToViewPos( in float3 pos) { return mul(pk_MATRIX_V, float4(pos, 1.0)).xyz; }

float3 ObjectToViewPos( in float3 pos) { return mul(pk_MATRIX_V, mul(pk_MATRIX_M, float4(pos, 1.0))).xyz; }

float3 ObjectToViewPos(float4 pos) { return ObjectToViewPos(pos.xyz); }

float3 ObjectToWorldPos( in float3 pos) { return mul(pk_MATRIX_M, float4(pos, 1.0)).xyz; }

float3 ObjectToWorldDir( in float3 dir) { return normalize(mul(float3x3(pk_MATRIX_M), dir)); }

float3 ObjectToWorldVector( in float3 dir) { return mul(float3x3(pk_MATRIX_M), dir); }

float3 ObjectToViewDir(float3 dir) { return normalize(mul(float3x3(pk_MATRIX_V), ObjectToWorldVector(dir))); }

float3 ObjectToWorldNormal( in float3 normal) { return normalize(mul(normal, float3x3(pk_MATRIX_I_M))); }

float3 WorldToObjectPos(in float3 pos) { return mul(pk_MATRIX_I_M, float4(pos, 1.0f)).xyz; }

float3 WorldToObjectVector( in float3 dir) { return mul(float3x3(pk_MATRIX_I_M), dir); }

float3 WorldToObjectDir( in float3 dir) { return normalize(mul(float3x3(pk_MATRIX_I_M), dir)); }

float4 ObjectToClipPos( in float3 pos) { return mul(pk_MATRIX_VP, mul(pk_MATRIX_M, float4(pos, 1.0))); }

float4 ObjectToClipPos(float4 pos) { return ObjectToClipPos(pos.xyz); }

float4 ClipToScreenPos(float4 clippos) 
{
    float4 screenpos = clippos * 0.5f;
    screenpos.xy = screenpos.xy + screenpos.w;
    screenpos.zw = clippos.zw;
    return screenpos;
}

float3 ClipToViewPos(float3 clippos)
{
    float3 v;
	v.x = pk_MATRIX_I_P[0][0] * clippos.x;
	v.y = pk_MATRIX_I_P[1][1] * clippos.y;
	v.z = LinearizeDepth(clippos.z);
	v.xy *= v.z;
    return v;
}

float3 ClipToViewPos(float2 uv, float linearDeth)
{
    return float3((uv * 2 - 1) * float2(pk_MATRIX_I_P[0][0], pk_MATRIX_I_P[1][1]), 1) * linearDeth;
}      

float3 ScreenToViewPos(float3 screenpos) 
{
    float2 texCoord = screenpos.xy / pk_ScreenParams.xy;
    return ClipToViewPos(float3(texCoord.xy * 2.0f - 1.0f, screenpos.z));
}

float3 ScreenToViewPos(float2 screenpos, float linearDepth) 
{
    float2 texCoord = screenpos.xy / pk_ScreenParams.xy;
    return ClipToViewPos(texCoord.xy, linearDepth);
}

#endif