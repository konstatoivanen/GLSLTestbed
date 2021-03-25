#pragma once
#ifndef MATH_UTILITIES
#define MATH_UTILITIES

#include HLSLSupport.glsl

//------------Macros------------//
#define overlay(s, d) (((d) < 0.5) ? 2.0 * (s) * (d) : 1.0 - 2.0 * (1.0 - (s)) * (1.0 - (d)))
#define unlerp(a,b,value) (((value) - (a)) / ((b) - (a)))
#define unlerp_sat(a,b,value) saturate(((value) - (a)) / ((b) - (a)))
#define mirror(u) (2 * abs(round(0.5 * (u)) - 0.5 * (u)))
#define rasterize(value, resolution) (((value) * (resolution)) / (resolution))
#define lengthsqr(x) dot(x,x)
#define grayscale(c) dot(c, float3(0.3, 0.59, 0.11))
#define safe_normalize(inVec) ((inVec) * rsqrt(max(0.001f, dot(inVec, inVec))))

//----------DECODING----------//
inline half2 decode_2H4S(float source)
{
    half2 value;
    uint suint = (uint)source;
    value.x = ((half) (suint & 0xFFFF) / 512) - 2;
    value.y = ((half) (suint >> 16) / 512) - 2;
    return value;
}

//----------NON ZERO SIGN----------//
float sgn(float x)
{
    return (x < 0) ? -1 : 1;
}

float2 sgn(float2 v)
{
    return float2(sgn(v.x) , sgn(v.y));
}

float3 sgn(float3 v)
{
    return float3(sgn(v.x), sgn(v.y), sgn(v.z));
}

float4 sgn(float4 v)
{
    return float4(sgn(v.x), sgn(v.y), sgn(v.z), sgn(v.w));
}

//----------SQUARE LENGTH----------//
float lengthSqr(float2 x)
{
    return dot(x, x);
}

float lengthSqr(float3 x)
{
    return dot(x, x);
}

float lengthSqr(float4 x)
{
    return dot(x, x);
}

//----------COMPONENT WISE MAX----------//
float cmax(float2 v)
{
    return max(v.x, v.y);
}

float cmax(float3 v)
{
    return max(max(v.x, v.y), v.z);
}

float cmax(float4 v)
{
    return max(max(v.x, v.y), max(v.z, v.w));
}

//----------COMPONENT WISE MIN----------//
float cmin(float2 v)
{
    return min(v.x, v.y);
}

float cmin(float3 v)
{
    return min(min(v.x, v.y), v.z);
}

float cmin(float4 v)
{
    return min(min(v.x, v.y), min(v.z, v.w));
}

//----------MOVE TOWARDS----------//
half2 movetowards(half2 current, half2 target, float maxDelta)
{
    half2 delta = target - current;
    half2 m = length(delta);

    delta = lerp(current + (delta / m) * maxDelta, target, step(m, maxDelta));

    return delta;
}

half3 movetowards(half3 current, half3 target, float maxDelta)
{
    half3 delta = target - current;
    half3 m = length(delta);

    delta = lerp(current + (delta / m) * maxDelta, target, step(m, maxDelta));

    return delta;
}

//----------COLOR CONVERSION----------//
float4 float_to_float4(float value)
{
    uint color32 = asuint(value);
    
    float4 color = 0;
    
    color.r = (color32 >> 24) & 0xFF;
    color.g = (color32 >> 16) & 0xFF;
    color.b = (color32 >> 8) & 0xFF;
    color.a = color32 & 0xFF;
    color /= 255.0f;
    
    return color;
}

float fixed4_to_float(fixed4 color)
{
    uint value = 0;
    
    color *= 255;
    
    uint4 color32 = (uint4)color;
    
    value |= color32.r << 24;
    value |= color32.g << 16;
    value |= color32.b << 8;
    value |= color32.a;
    return asfloat(value);
}

//----------DIRECTION UTILITIES----------//
float directionRadian(float2 direction)
{
    return acos(clamp(dot(float2(1, 0), direction), -1.0, 1.0));
}

half2 radianDirection(half radian)
{
    return half2(cos(radian), sin(radian));
}

half2 rotate(half2 dir, half radian)
{
    float _cos = cos(radian);
    float _sin = sin(radian);
    return half2(dir.x * _cos - dir.y * _sin, dir.x * _sin + dir.y * _cos);
}

//----------DEPTH CONVERSION----------//
half depth_to_distance(float depth, float4 projectionParameters)
{
    half2 ab = half2(projectionParameters.z / (projectionParameters.z - projectionParameters.y), projectionParameters.z * projectionParameters.y / (projectionParameters.y - projectionParameters.z));
    return ab.y / (1.0 - depth - ab.x);
}

half2 depth_to_distance(float2 depth, float4 projectionParameters)
{
    half2 ab = half2(projectionParameters.z / (projectionParameters.z - projectionParameters.y), projectionParameters.z * projectionParameters.y / (projectionParameters.y - projectionParameters.z));
    return ab.y / (1.0 - depth - ab.x);
}

//----------UV UTILITIES----------//
float2 kaleidouv(float2 uv, float divisor, float offset, float roll)
{
		// Convert to the polar coordinate.
    float2 sc = uv - 0.5;
    float phi = atan2(sc.y, sc.x);
    float r = sqrt(dot(sc, sc));

		// Angular repeating.
    phi += offset;
    phi = phi - divisor * floor(phi / divisor);
    phi += roll - offset;

		// Convert back to the texture coordinate.
    uv = float2(cos(phi), sin(phi)) * r + 0.5;

		// Reflection at the border of the screen.
    uv = max(min(uv, 2.0 - uv), -uv);

    return uv;
}

#endif // MATH_CG_INCLUDED