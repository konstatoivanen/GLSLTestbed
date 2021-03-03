#ifndef PK_Lighting
#define PK_Lighting

#include PKCommon.glsl

uniform sampler2D pk_SceneOEM_HDR;
uniform float4 pk_SceneOEM_ST;
uniform float pk_SceneOEM_RVS[3];

#define HDRFactor 4.0

float4 HDREncode(float4 color)
{
    return float4(color.rgb / HDRFactor, color.a);
}

float4 HDRDecode(float4 hdr)
{
    return float4(hdr.rgb * HDRFactor, hdr.a);
}

float2 OctaEncode(float3 n)
{
    float4 a;
    float2 b;
    bool3 c;

    a.x = abs(n.y) + abs(n.x);
    a.x = a.x + abs(n.z);
    a.xyz = n.yxz / a.xxx;
    b.xy = -abs(a.zy) + 1.0f;
    c.xyz = greaterThanEqual(a.xyz, float3(0.0, 0.0, 0.0)).xyz;
    a.x = (c.y) ? 1.0f : -1.0f;
    a.w = (c.z) ? 1.0f : -1.0f;
    a.xw = a.xw * b.xy;
    a.xy = (c.x) ? a.yz : a.xw;
    return a.xy * 0.5f + 0.5f;
}

float3 OctaDecode(float2 f)
{
    f = f * 2.0f - 1.0f;

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, 1.0f - abs(f.x) - abs(f.y), f.y);
    float t = max(-n.y, 0.0);
    n.x += n.x >= 0.0f ? -t : t;
    n.z += n.z >= 0.0f ? -t : t;
    return normalize(n);
}

float2 OctaUV(float3 reflection)
{
    float2 reflUV = OctaEncode(reflection);
    reflUV = pk_SceneOEM_ST.xy + reflUV * pk_SceneOEM_ST.z;
    return reflUV;
}

float2 OctaUV(float2 offset, float3 reflection)
{
    float2 reflUV = OctaEncode(reflection);
    reflUV = offset + reflUV * pk_SceneOEM_ST.z;
    return reflUV;
}

float3 SampleEnv(float2 uv, float roughness)
{
    float v0 = saturate((roughness - pk_SceneOEM_RVS[0]) / (pk_SceneOEM_RVS[1] - pk_SceneOEM_RVS[0]));
    float v1 = saturate((roughness - pk_SceneOEM_RVS[1]) / (pk_SceneOEM_RVS[2] - pk_SceneOEM_RVS[1]));
    float4 env = tex2DLod(pk_SceneOEM_HDR, uv, v0 + v1);
    return HDRDecode(env).rgb;
}
#endif