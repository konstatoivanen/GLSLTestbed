#ifndef PK_Lighting
#define PK_Lighting

#include HLSLSupport.glsl

// SH lighting environment
uniform float4 pk_SHAr;
uniform float4 pk_SHAg;
uniform float4 pk_SHAb;
uniform float4 pk_SHBr;
uniform float4 pk_SHBg;
uniform float4 pk_SHBb;
uniform float4 pk_SHC;

// normal should be normalized, w=1.0
float3 SHEvalLinearL0L1(float4 normal)
{
    float3 x;
    // Linear (L1) + constant (L0) polynomial terms
    x.r = dot(pk_SHAr, normal);
    x.g = dot(pk_SHAg, normal);
    x.b = dot(pk_SHAb, normal);

    return x;
}

// normal should be normalized, w=1.0
float3 SHEvalLinearL2(float4 normal)
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
float3 ShadeSH9(float4 normal)
{
    // Linear + constant polynomial terms
    float3 res = SHEvalLinearL0L1(normal);

    // Quadratic polynomials
    res += SHEvalLinearL2(normal);

#ifdef PK_COLORSPACE_GAMMA
    res = LinearToGammaSpace(res);
#endif

    return res;
}
#endif