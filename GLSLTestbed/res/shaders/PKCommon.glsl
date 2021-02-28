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

layout(std140) uniform pk_PerFrameConstants
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
    float3 pk_WorldSpaceCameraPos;
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
    layout(std430) buffer pk_InstancingData
    {
    	float4x4 pk_InstancingMatrices[];
    };
    #define ACTIVE_MODEL_MATRIX pk_InstancingMatrices[gl_InstanceID]
#else
    #define ACTIVE_MODEL_MATRIX pk_MATRIX_M
#endif


// SH lighting environment
uniform float4 pk_SHAr;
uniform float4 pk_SHAg;
uniform float4 pk_SHAb;
uniform float4 pk_SHBr;
uniform float4 pk_SHBg;
uniform float4 pk_SHBb;
uniform float4 pk_SHC;

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

float3 ObjectToViewPos(float4 pos) // overload for float4; avoids "implicit truncation" warning for existing shaders
{
    return ObjectToViewPos(pos.xyz);
}

// Tranforms position from world to camera space
float3 WorldToViewPos( in float3 pos)
{
    return mul(pk_MATRIX_V, float4(pos, 1.0)).xyz;
}

// Transforms direction from object to world space
float3 ObjectToWorldDir( in float3 dir)
{
    return normalize(mul(float3x3(ACTIVE_MODEL_MATRIX), dir));
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

// normal should be normalized, w=1.0
float3 SHEvalLinearL0L1 (float4 normal)
{
    half3 x;

    // Linear (L1) + constant (L0) polynomial terms
    x.r = dot(pk_SHAr, normal);
    x.g = dot(pk_SHAg, normal);
    x.b = dot(pk_SHAb, normal);

    return x;
}

// normal should be normalized, w=1.0
float3 SHEvalLinearL2 (float4 normal)
{
    float3 x1, x2;
    // 4 of the quadratic (L2) polynomials
    float4 vB = normal.xyzz * normal.yzzx;
    x1.r = dot(pk_SHBr,vB);
    x1.g = dot(pk_SHBg,vB);
    x1.b = dot(pk_SHBb,vB);

    // Final (5th) quadratic (L2) polynomial
    float vC = normal.x * normal.x - normal.y * normal.y;
    x2 = pk_SHC.rgb * vC;

    return x1 + x2;
}

// normal should be normalized, w=1.0
// output in active color space
float3 ShadeSH9 (float4 normal)
{
    // Linear + constant polynomial terms
    float3 res = SHEvalLinearL0L1 (normal);

    // Quadratic polynomials
    res += SHEvalLinearL2 (normal);

#ifdef PK_COLORSPACE_GAMMA
    res = LinearToGammaSpace (res);
#endif

    return res;
}
#endif