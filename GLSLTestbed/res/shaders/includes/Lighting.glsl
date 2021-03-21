#pragma once
#include PKCommon.glsl
#include LightingCommon.glsl
#include LightingBRDF.glsl

uniform sampler2D pk_ScreenOcclusion;
uniform sampler2D pk_SceneOEM_HDR;
uniform float4 pk_SceneOEM_ST;
uniform float pk_SceneOEM_RVS[4];
uniform float pk_SceneOEM_Exposure;

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
    //float v0 = saturate((roughness - pk_SceneOEM_RVS[0]) / (pk_SceneOEM_RVS[1] - pk_SceneOEM_RVS[0]));
    //float v1 = saturate((roughness - pk_SceneOEM_RVS[1]) / (pk_SceneOEM_RVS[2] - pk_SceneOEM_RVS[1]));
    float4 env = tex2DLod(pk_SceneOEM_HDR, uv, roughness * 4);
    return HDRDecode(env).rgb * pk_SceneOEM_Exposure;
}

float SampleScreenSpaceOcclusion(float2 uv)
{
    return 1.0f - tex2D(pk_ScreenOcclusion, uv).r;
}

PKLight TransformPointLight(in PKRawPointLight raw, in float3 worldpos)
{
    float3 vector =  raw.position.xyz - worldpos;
    float sqrdist = dot(vector, vector);
    float atten = max(1.0f - sqrdist / (raw.position.w * raw.position.w), 0.0);
    PKLight l;
    l.color = raw.color.xyz * atten;
    l.direction = vector / sqrt(sqrdist);
    return l;
}

float4 PhysicallyBasedShading(SurfaceData surf, float3 viewdir, float3 worldpos)
{
    surf.normal = normalize(surf.normal);
    surf.roughness = max(surf.roughness, 0.002);

    // Fix edge artifacts for when normals are pointing away from camera.
    half shiftAmount = dot(surf.normal, viewdir);
    surf.normal = shiftAmount < 0.0f ? surf.normal + viewdir * (-shiftAmount + 1e-5f) : surf.normal;

    float3 specColor = lerp(pk_DielectricSpecular.rgb, surf.albedo, surf.metallic);
    float reflectivity = pk_DielectricSpecular.r + surf.metallic * pk_DielectricSpecular.a;
    surf.albedo *= 1.0f - reflectivity;

    // this is necessary to handle transparency in physically correct way - only diffuse component gets affected by alpha
    #if defined(PK_TRANSPARENT_PBR)
        surf.albedo *= alpha;
        surf.alpha = reflectivity + surf.alpha * (1.0f - reflectivity);
    #endif

    PKGI gi;

    float2 specUV = OctaUV(reflect(-viewdir, surf.normal));
    float2 diffUV = OctaUV(surf.normal);

    gi.indirect.diffuse.xyz  = SampleEnv(diffUV, 1.0f);
    gi.indirect.specular.xyz = SampleEnv(specUV, surf.roughness);
    
    float3 color = BRDF_PBS_DEFAULT(surf.albedo, specColor, reflectivity, surf.roughness, surf.normal, viewdir, gi.light, gi.indirect);

    color *= surf.occlusion;

    gi.indirect = EmptyIndirect();

    for (uint i = 0; i < pk_LightCount; ++i)
    {
        gi.light = TransformPointLight(PK_BUFFER_DATA(pk_Lights, i), worldpos);
        color += BRDF_PBS_DEFAULT(surf.albedo, specColor, reflectivity, surf.roughness, surf.normal, viewdir, gi.light, gi.indirect);
    }

    return float4(color, surf.alpha);
}
