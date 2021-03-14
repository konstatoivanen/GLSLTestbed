#pragma once
#ifndef PK_COMMON
#define PK_COMMON

#include HLSLSupport.glsl

#define PK_PI            3.14159265359f
#define PK_TWO_PI        6.28318530718f
#define PK_FOUR_PI       12.56637061436f
#define PK_INV_PI        0.31830988618f
#define PK_INV_TWO_PI    0.15915494309f
#define PK_INV_FOUR_PI   0.07957747155f
#define PK_HALF_PI       1.57079632679f
#define PK_INV_HALF_PI   0.636619772367f

#define pk_ColorSpaceGrey fixed4(0.214041144, 0.214041144, 0.214041144, 0.5)
#define pk_ColorSpaceDouble fixed4(4.59479380, 4.59479380, 4.59479380, 2.0)
#define pk_ColorSpaceDielectricSpec half4(0.04, 0.04, 0.04, 1.0 - 0.04) // standard dielectric reflectivity coef at incident angle (= 4%)
#define pk_ColorSpaceLuminance half4(0.0396819152, 0.458021790, 0.00609653955, 1.0)

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
    // x is 1.0 (or –1.0 if currently rendering with a flipped projection matrix), y is the camera’s near plane, z is the camera’s far plane and w is 1/FarPlane.
    float4 pk_ProjectionParams;
    // x is the width of the camera’s target texture in pixels, y is the height of the camera’s target texture in pixels, z is 1.0 + 1.0/width and w is 1.0 + 1.0/height.
    float4 pk_ScreenParams;
    // Used to linearize Z buffer values. x is (1-far/near), y is (far/near), z is (x/far) and w is (y/far).
    float4 pk_ZBufferParams;
    
    // Current view matrix.
    float4x4 pk_MATRIX_V;
    // Current projection matrix.
    float4x4 pk_MATRIX_P;
    // Current view * projection matrix.
    float4x4 pk_MATRIX_VP;
    // Current inverse view * projection matrix.
    float4x4 pk_MATRIX_I_VP;
};

// Current model matrix.
uniform float4x4 pk_MATRIX_M;
// Current inverse model matrix.
uniform float4x4 pk_MATRIX_I_M;

#if defined(PK_ENABLE_INSTANCING) && defined(SHADER_STAGE_VERTEX)
    PK_DECLARE_BUFFER(float4x4, pk_InstancingData);
    uniform int pk_InstancingOffset;
    #define ACTIVE_MODEL_MATRIX PK_BUFFER_DATA(pk_InstancingData, PK_INSTANCE_ID + pk_InstancingOffset)
#else
    #define ACTIVE_MODEL_MATRIX pk_MATRIX_M
#endif

// An almost-perfect approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
float3 LinearToGammaSpace (float3 linRGB)
{
    linRGB = max(linRGB, half3(0.0f, 0.0f, 0.0f));
    return max(1.055f * pow(linRGB, (0.416666667).xxx) - 0.055f, 0.0f);
}

#define HDRFactor 4.0

float4 HDREncode(float4 color)
{
    return float4(color.rgb / HDRFactor, color.a);
}

float4 HDRDecode(float4 hdr)
{
    return float4(hdr.rgb * HDRFactor, hdr.a);
}

float3 UnpackNormal(in float3 packedNormal)
{
    return packedNormal * 2.0f - float3(1.0f);
}

float3 SampleNormal(in sampler2D map, in float3x3 rotation, in float2 uv, float amount)
{
    return lerp(rotation[2], mul(rotation, UnpackNormal(tex2D(map, uv).xyz)), amount);
}

float2 ParallaxOffset(float height, float heightAmount, float3 viewdir)
{
    return (height * heightAmount - heightAmount / 2.0f) * (viewdir.xy / (viewdir.z + 0.42f));
}

// Tranforms position from world to homogenous space
float4 WorldToClipPos( in float3 pos)
{
    return mul(pk_MATRIX_VP, float4(pos, 1.0));
}

// Tranforms position from view to homogenous space
float4 ViewToClipPos( in float3 pos)
{
    return mul(pk_MATRIX_P, float4(pos, 1.0));
}

// Tranforms position from object to camera space
float3 ObjectToViewPos( in float3 pos)
{
    return mul(pk_MATRIX_V, mul(ACTIVE_MODEL_MATRIX, float4(pos, 1.0))).xyz;
}

// Tranforms position from object to camera space
float3 ObjectToViewPos(float4 pos)
{
    return ObjectToViewPos(pos.xyz);
}

// Tranforms position from world to camera space
float3 WorldToViewPos( in float3 pos)
{
    return mul(pk_MATRIX_V, float4(pos, 1.0)).xyz;
}

// Transforms position from object to world space
float3 ObjectToWorldPos( in float3 pos)
{
    return mul(ACTIVE_MODEL_MATRIX, float4(pos, 1.0)).xyz;
}

// Transforms direction from object to world space
float3 ObjectToWorldDir( in float3 dir)
{
    return normalize(mul(float3x3(ACTIVE_MODEL_MATRIX), dir));
}

// Transforms vector from world to object space
float3 WorldToObjectPos(in float3 pos)
{
    return mul(pk_MATRIX_I_M, float4(pos, 1.0f)).xyz;
}

// Transforms vector from world to object space
float3 WorldToObjectVector( in float3 dir)
{
    return mul(float3x3(pk_MATRIX_I_M), dir);
}

// Transforms vector from object to world space
float3 ObjectToWorldVector( in float3 dir)
{
    return mul(float3x3(ACTIVE_MODEL_MATRIX), dir);
}

// Transforms direction from world to object space
float3 WorldToObjectDir( in float3 dir)
{
    return normalize(mul(float3x3(pk_MATRIX_I_M), dir));
}

// Transforms normal from object to world space
float3 ObjectToWorldNormal( in float3 norm)
{
#ifdef PK_ASSUME_UNIFORM_SCALING
    return ObjectToWorldDir(norm);
#else
    // mul(IT_M, norm) => mul(norm, I_M) => {dot(norm, I_M.col0), dot(norm, I_M.col1), dot(norm, I_M.col2)}
    return normalize(mul(norm, float3x3(pk_MATRIX_I_M)));
#endif
}

// Tranforms position from object to homogenous space
float4 ObjectToClipPos( in float3 pos)
{
    return mul(pk_MATRIX_VP, mul(ACTIVE_MODEL_MATRIX, float4(pos, 1.0)));
}

// overload for float4; avoids "implicit truncation" warning for existing shaders
float4 ObjectToClipPos(float4 pos)
{
    return ObjectToClipPos(pos.xyz);
}

#endif