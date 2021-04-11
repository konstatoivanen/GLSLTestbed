#pragma once
#include PKCommon.glsl
#include ClusterIndexing.glsl
#include LightingCommon.glsl
#include LightingBRDF.glsl

uniform sampler2D pk_ShadowmapAtlas;
uniform sampler2D pk_ScreenOcclusion;
uniform sampler2D pk_SceneOEM_HDR;
uniform float pk_SceneOEM_Exposure;

#define SRC_METALLIC x
#define SRC_OCCLUSION y
#define SRC_ROUGHNESS z
#define SHADOW_USE_LBR 
#define SHADOW_LBR 0.1f
#define SHADOWMAP_TILE_SIZE 512
#define SHADOWMAP_ATLAS_SIZE 4096
#define SHADOWMAP_TILES_PER_AXIS 8
#define SHADOWMAP_BORDER_SIZE (1.0f / SHADOWMAP_ATLAS_SIZE)

// Source: https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
float GetAttenuation(float ldist, float lradius) { return pow2(saturate(1.0f - pow4(ldist/lradius))) / (pow2(ldist) + 1.0f); }

#if defined(SHADOW_USE_LBR)
    float LBR(float shadow) { return smoothstep(SHADOW_LBR, 1.0f, shadow);}
#else
    #define LBR(shadow) (shadow)
#endif

float3 GetShadowMapTileST(uint index)
{
    return float3(
    uint(index % SHADOWMAP_TILES_PER_AXIS) / float(SHADOWMAP_TILES_PER_AXIS) + SHADOWMAP_BORDER_SIZE, 
    uint(index / SHADOWMAP_TILES_PER_AXIS) / float(SHADOWMAP_TILES_PER_AXIS) + SHADOWMAP_BORDER_SIZE, 
    1.0f / SHADOWMAP_TILES_PER_AXIS - 2 * SHADOWMAP_BORDER_SIZE);
}

float2 OctaWrap(float2 v)
{
    return (1.0 - abs(v.yx)) * float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

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

float2 OctaUV(float3 offset, float3 reflection)
{
    return offset.xy + OctaEncode(reflection) * offset.z;
}

float2 OctaUV(float3 reflection)
{
    return OctaEncode(reflection);
}


float3 SampleEnv(float2 uv, float roughness)
{
    float4 env = tex2DLod(pk_SceneOEM_HDR, uv, roughness * 4);
    return HDRDecode(env) * pk_SceneOEM_Exposure;
}

float SampleLightShadowmap(uint index, float3 direction, float lightDistance)
{
    float3 shadowST = GetShadowMapTileST(index);
    float2 shadowval = tex2D(pk_ShadowmapAtlas, OctaUV(shadowST, direction)).xy;
    float variance = shadowval.y - shadowval.x * shadowval.x;
    float difference = lightDistance - shadowval.x;
    return difference > 0.01f ? LBR(variance / (variance + difference * difference)) : 1.0f;
}

float SampleScreenSpaceOcclusion(float2 uv)
{
    return 1.0f - tex2D(pk_ScreenOcclusion, uv).r;
}

float SampleScreenSpaceOcclusion()
{
    #if defined(SHADER_STAGE_FRAGMENT)
        return 1.0f - tex2D(pk_ScreenOcclusion, (gl_FragCoord.xy / pk_ScreenParams.xy)).r;
    #else
        return 1.0f;
    #endif
}

LightTile GetLightTile(uint index)
{
    #if defined(SHADER_STAGE_FRAGMENT)
        uint data = PK_BUFFER_DATA(pk_LightTiles, index);
        uint offset = data & 0xFFFFFF;
        return LightTile(offset, offset + (data >> 24));
    #else
        return LightTile(0,0);
    #endif
}

LightTile GetLightTile()
{
    return GetLightTile(GetTileIndexFragment());
}

PKLight GetSurfaceLight(uint index, in float3 worldpos)
{
    uint linearIndex = PK_BUFFER_DATA(pk_GlobalLightsList, index);
    PKRawLight raw = PK_BUFFER_DATA(pk_Lights, linearIndex);

    float3 vector = raw.position.xyz - worldpos;
    float lindist = sqrt(dot(vector, vector));
    float atten = GetAttenuation(lindist, raw.position.w);
    vector /= lindist;

    atten *= SampleLightShadowmap(raw.shadowmap_index, -vector, lindist);

    PKLight l;
    l.color = raw.color.xyz * atten;
    l.direction = vector;

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

    float2 specUV = OctaUV(reflect(-viewdir, surf.normal));
    float2 diffUV = OctaUV(surf.normal);

    PKIndirect indirect;
    indirect.diffuse.xyz  = SampleEnv(diffUV, 1.0f);
    indirect.specular.xyz = SampleEnv(specUV, surf.roughness);

    INIT_BRDF_CACHE(surf.albedo, specColor, surf.normal, viewdir, reflectivity, surf.roughness);
    
    float3 color = BRDF_PBS_DEFAULT_INDIRECT(indirect);

    color *= surf.occlusion;

    LightTile tile = GetLightTile();

    for (uint i = tile.start; i < tile.end; ++i)
    {
        PKLight light = GetSurfaceLight(i, worldpos);
        color += BRDF_PBS_DEFAULT_DIRECT(light);
    }

    return float4(color, surf.alpha);
}
