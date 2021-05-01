#pragma once
#include HLSLSupport.glsl

#define HDRFactor 8.0

float4 HDREncode(float3 color) 
{
    color /= HDRFactor;
    float alpha = ceil( max(max(color.r, color.g), color.b) * 255.0) / 255.0;
    return float4(color / alpha, alpha);
}

float3 HDRDecode(float4 hdr) { return float3(hdr.rgb * hdr.a * HDRFactor); }

float2 OctaWrap(float2 v) { return (1.0 - abs(v.yx)) * float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0); }

float2 OctaEncode(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xz = n.y >= 0.0 ? n.xz : OctaWrap(n.xz);
    n.xz = n.xz * 0.5 + 0.5;
    return n.xz;
}

//Source: https://twitter.com/Stubbesaurus/status/937994790553227264
float3 OctaDecode(float2 f)
{
    f = f * 2.0f - 1.0f;
    float3 n = float3(f.x, 1.0f - abs(f.x) - abs(f.y), f.y);
    float t = max(-n.y, 0.0);
    n.x += n.x >= 0.0f ? -t : t;
    n.z += n.z >= 0.0f ? -t : t;
    return normalize(n);
}

float2 OctaUV(float3 offset, float3 direction) { return offset.xy + OctaEncode(direction) * offset.z; }

float2 OctaUV(float3 direction) { return OctaEncode(direction); }