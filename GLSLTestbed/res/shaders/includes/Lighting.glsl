#pragma once
#include PKCommon.glsl
#include ClusterIndexing.glsl
#include LightingCommon.glsl
#include LightingBRDF.glsl

#define SRC_METALLIC x
#define SRC_OCCLUSION y
#define SRC_ROUGHNESS z
#define SHADOW_USE_LBR 
#define SHADOW_LBR 0.1f
#define SHADOWMAP_CASCADE_LINEARITY 0.5f
#define SHADOWMAP_CASCADES 4
#define SHADOWMAP_TILE_SIZE 512
#define SHADOWMAP_ATLAS_SIZE 4096
#define SHADOWMAP_TILES_PER_AXIS 8
#define SHADOWMAP_BORDER_SIZE (1.0f / SHADOWMAP_ATLAS_SIZE)
uniform float4 pk_ShadowCascadeZSplits;

#if defined(SHADOW_USE_LBR)
    float LBR(float shadow) { return smoothstep(SHADOW_LBR, 1.0f, shadow);}
#else
    #define LBR(shadow) (shadow)
#endif

// Source: https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
float GetAttenuation(float ldist, float lradius) { return pow2(saturate(1.0f - pow4(ldist/lradius))) / (pow2(ldist) + 1.0f); }

float GetLightAnisotropy(float3 worldPos, float3 posToLight, float anistropy)
{
    float3 cameraToPos = normalize(worldPos - pk_WorldSpaceCameraPos.xyz);
	float g = anistropy;
	float gsq = g * g;
	float denom = 1 + gsq - 2.0 * g * dot(cameraToPos, posToLight);
	denom = denom * denom * denom;
	denom = sqrt(max(0, denom));
	return (1 - gsq) / denom;
}

uint GetShadowCascadeIndex(float linearDepth)
{
    for (uint i = SHADOWMAP_CASCADES - 1; i > 0; --i)
    {
        if (linearDepth > pk_ShadowCascadeZSplits[i])
        {
            return i;
        }
    }

    return 0;
}

uint GetShadowCascadeIndexFragment()
{
    #if defined(SHADER_STAGE_FRAGMENT)
        return GetShadowCascadeIndex(LinearizeDepth(gl_FragCoord.z));
    #else
        return 0u;
    #endif
}

float3 GetShadowMapPaddedTileST(uint index)
{
    return float3(
    uint(index % SHADOWMAP_TILES_PER_AXIS) / float(SHADOWMAP_TILES_PER_AXIS) + SHADOWMAP_BORDER_SIZE, 
    uint(index / SHADOWMAP_TILES_PER_AXIS) / float(SHADOWMAP_TILES_PER_AXIS) + SHADOWMAP_BORDER_SIZE, 
    1.0f / SHADOWMAP_TILES_PER_AXIS - 2 * SHADOWMAP_BORDER_SIZE);
}

float3 GetShadowMapTileST(uint index)
{
    return float3(
    uint(index % SHADOWMAP_TILES_PER_AXIS) / float(SHADOWMAP_TILES_PER_AXIS), 
    uint(index / SHADOWMAP_TILES_PER_AXIS) / float(SHADOWMAP_TILES_PER_AXIS), 
    1.0f / SHADOWMAP_TILES_PER_AXIS);
}

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


float3 SampleEnv(float2 uv, float roughness)
{
    float4 env = tex2DLod(pk_SceneOEM_HDR, uv, roughness * 4);
    return HDRDecode(env) * pk_SceneOEM_Exposure;
}

float SampleScreenSpaceOcclusion(float2 uv) { return 1.0f - tex2D(pk_ScreenOcclusion, uv).r; }

float SampleScreenSpaceOcclusion()
{
    #if defined(SHADER_STAGE_FRAGMENT)
        return SampleScreenSpaceOcclusion(gl_FragCoord.xy / pk_ScreenParams.xy);
    #else
        return 1.0f;
    #endif
}

float3 GetLightUV(PKRawLight light, float3 worldpos, float3 direction, uint cascade)
{
    switch(light.type)
    {
        case LIGHT_TYPE_POINT:
        {
            return float3(OctaEncode(direction), 1.0f);
        }
        case LIGHT_TYPE_SPOT:
        {
            float4x4 matrix = PK_BUFFER_DATA(pk_LightMatrices, light.projection_index);
            float4 coord = mul(matrix, float4(worldpos, 1.0f));
            coord = ClipToScreenPos(coord);
            return float3(coord.xy / coord.w, step(0.0f, coord.z * 0.5f));
        }
        case LIGHT_TYPE_DIRECTIONAL: 
        {
            float4x4 matrix = PK_BUFFER_DATA(pk_LightMatrices, light.projection_index + cascade);
            float4 coord = mul(matrix, float4(worldpos, 1.0f));
            coord = ClipToScreenPos(coord);
            return float3(coord.xy / coord.w, 1.0f);
        }
    }

    return float3(0.0f);
}

float SampleLightShadowmap(PKRawLight light, float2 uv, float lightDistance, uint cascade)
{
    if (light.shadowmap_index == LIGHT_PARAM_INVALID)
    {
        return 1.0f;
    }

    cascade = light.type == LIGHT_TYPE_DIRECTIONAL ? cascade : 0;
    float3 shadowST = GetShadowMapPaddedTileST(light.shadowmap_index + cascade);
    float2 moments = tex2D(pk_ShadowmapAtlas, shadowST.xy + uv * shadowST.z).xy;
    float variance = moments.y - moments.x * moments.x;
    float difference = lightDistance - moments.x;
    return difference > 0.01f ? LBR(variance / (variance + difference * difference)) : 1.0f;
}

float SampleLightCookie(PKRawLight light, float2 uv)
{
    if (light.cookie_index == LIGHT_PARAM_INVALID)
    {
        return 1.0f;
    }

    return tex2D(pk_LightCookies, float3(uv, float(light.cookie_index))).r;
}

void GetLightParams(PKRawLight light, float3 worldpos, out float3 posToLight, out float lindist, out float attenuation)
{
    if (light.type == LIGHT_TYPE_DIRECTIONAL)
    {
        posToLight = -light.position.xyz;
        lindist = dot(light.position.xyz, worldpos) + light.position.w;
        attenuation = 1.0f;
        return;
    }

    posToLight = light.position.xyz - worldpos;
    lindist = sqrt(dot(posToLight, posToLight));
    attenuation = GetAttenuation(lindist, light.position.w);
    posToLight /= lindist;
}

PKLight GetSurfaceLight(uint index, in float3 worldpos, uint cascade)
{
    uint linearIndex = PK_BUFFER_DATA(pk_GlobalLightsList, index);
    PKRawLight light = PK_BUFFER_DATA(pk_Lights, linearIndex);

    float3 posToLight;
    float lindist;
    float attenuation;
    GetLightParams(light, worldpos, posToLight, lindist, attenuation);

    float3 lightuvw = GetLightUV(light, worldpos, -posToLight, cascade);

    attenuation *= lightuvw.z;
    attenuation *= SampleLightCookie(light, lightuvw.xy);
    attenuation *= SampleLightShadowmap(light, lightuvw.xy, lindist, cascade);

    PKLight outlight;
    outlight.color = light.color.xyz * attenuation;
    outlight.direction = posToLight;
    return outlight;
}

float3 GetVolumeLightColor(uint index, in float3 worldpos, uint cascade, float anisotropy)
{
    uint linearIndex = PK_BUFFER_DATA(pk_GlobalLightsList, index);
    PKRawLight light = PK_BUFFER_DATA(pk_Lights, linearIndex);

    float3 posToLight;
    float lindist;
    float attenuation;
    GetLightParams(light, worldpos, posToLight, lindist, attenuation);

    float3 lightuvw = GetLightUV(light, worldpos, -posToLight, cascade);

    attenuation *= lightuvw.z;
	attenuation *= GetLightAnisotropy(worldpos, posToLight, anisotropy);
    attenuation *= SampleLightCookie(light, lightuvw.xy);
    attenuation *= SampleLightShadowmap(light, lightuvw.xy, lindist, cascade);

    return light.color.xyz * attenuation;
}

LightTile GetLightTile(int3 coord)
{
    #if defined(PK_WRITE_CLUSTER_LIGHTS)
        return LightTile(0,0);
    #else
        return CreateLightTile(imageLoad(pk_LightTiles, coord).x);
    #endif
}

LightTile GetLightTile() { return GetLightTile(GetTileIndexFragment()); }


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

    uint cascade = GetShadowCascadeIndexFragment();
    LightTile tile = GetLightTile();

    for (uint i = tile.start; i < tile.end; ++i)
    {
        PKLight light = GetSurfaceLight(i, worldpos, cascade);
        color += BRDF_PBS_DEFAULT_DIRECT(light);
    }

    return float4(color, surf.alpha);
}
