#pragma once
#include PKCommon.glsl
#include ClusterIndexing.glsl
#include LightingCommon.glsl
#include LightingBRDF.glsl
#include Encoding.glsl

#define SRC_METALLIC x
#define SRC_OCCLUSION y
#define SRC_ROUGHNESS z
#define SHADOW_USE_LBR 
#define SHADOW_LBR 0.2f
#define SHADOWMAP_CASCADES 4

#if defined(SHADOW_USE_LBR)
    float LBR(float shadow) { return smoothstep(SHADOW_LBR, 1.0f, shadow);}
#else
    #define LBR(shadow) (shadow)
#endif

// Source: https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
float GetAttenuation(float ldist, float lradius) { return pow2(saturate(1.0f - pow4(ldist/lradius))) / (pow2(ldist) + 1.0f); }

float GetLightAnisotropy(float3 viewdir, float3 posToLight, float anistropy)
{
	float gsq = pow2(anistropy);
	float denom = 1.0 + gsq - 2.0 * anistropy * dot(viewdir, posToLight);
	return (1.0 - gsq) * inversesqrt(max(0, pow3(denom)));
}

uint GetShadowCascadeIndexFragment()
{
    #if defined(SHADER_STAGE_FRAGMENT)
        return GetShadowCascadeIndex(LinearizeDepth(gl_FragCoord.z));
    #else
        return 0u;
    #endif
}

float3 SampleEnv(float2 uv, float roughness) { return HDRDecode(tex2DLod(pk_SceneOEM_HDR, uv, roughness * 4)) * pk_SceneOEM_Exposure; }

float SampleScreenSpaceOcclusion(float2 uv) { return tex2D(pk_ScreenOcclusion, uv).r; }

float SampleScreenSpaceOcclusion()
{
    #if defined(SHADER_STAGE_FRAGMENT)
        return SampleScreenSpaceOcclusion(gl_FragCoord.xy * pk_ScreenParams.zw);
    #else
        return 1.0f;
    #endif
}

void SampleScreenSpaceGI(inout PKIndirect indirect)
{
    #if defined(SHADER_STAGE_FRAGMENT)
        float2 screenuv = gl_FragCoord.xy * pk_ScreenParams.zw;

        float4 diffuse = tex2D(pk_ScreenGI_Diffuse, screenuv);
        float4 specular = tex2D(pk_ScreenGI_Specular, screenuv);

        indirect.diffuse = indirect.diffuse * diffuse.a + diffuse.rgb;
        indirect.specular = indirect.specular * specular.a + specular.rgb;
    #endif
}


float4 GetLightProjectionUVW(in float3 worldpos, uint projectionIndex)
{
    float4 coord = mul(PK_BUFFER_DATA(pk_LightMatrices, projectionIndex), float4(worldpos, 1.0f));
    coord.xy = (coord.xy * 0.5f + coord.ww * 0.5f) / coord.w;
    return coord;
}

float SampleLightShadowmap(uint shadowmapIndex, float2 uv, float lightDistance)
{
    float2 moments = tex2D(pk_ShadowmapArray, float3(uv, shadowmapIndex)).xy;
    float variance = moments.y - moments.x * moments.x;
    float difference = lightDistance - moments.x;
    return difference > 0.01f ? LBR(variance / (variance + difference * difference)) : 1.0f;
}

void GetLight(uint index, in float3 worldpos, uint cascade, out float3 color, out float3 posToLight, out float attenuation)
{
    PKRawLight light = PK_BUFFER_DATA(pk_Lights, index);
    color = light.color.rgb;

    float2 lightuv;
    float linearDistance;

    // @TODO Maybe refactor lights to separate by type lists 
    switch (light.type)
    {
        case LIGHT_TYPE_POINT:
        {
            posToLight = light.position.xyz - worldpos;
            linearDistance = length(posToLight);
            attenuation = GetAttenuation(linearDistance, light.position.w);
            posToLight /= linearDistance;
            lightuv = OctaEncode(-posToLight);
        }
        break;
        case LIGHT_TYPE_SPOT:
        {
            posToLight = light.position.xyz - worldpos;
            linearDistance = length(posToLight);
            attenuation = GetAttenuation(linearDistance, light.position.w);
            posToLight /= linearDistance;

            float3 coord = GetLightProjectionUVW(worldpos, light.projection_index).xyz;
            lightuv = coord.xy;
            attenuation *= step(0.0f, coord.z);
            attenuation *= tex2D(pk_LightCookies, float3(lightuv, light.cookie_index)).r;
        }
        break;
        case LIGHT_TYPE_DIRECTIONAL:
        {
            light.projection_index += cascade;
            light.shadowmap_index += cascade;
            posToLight = -light.position.xyz;
            attenuation = 1.0f;

            float4 coord = GetLightProjectionUVW(worldpos, light.projection_index);
            linearDistance = ((coord.z / coord.w) + 1.0f) * light.position.w * 0.5f;
            lightuv = coord.xy;
        }
        break;
    }

    //if (light.cookie_index != LIGHT_PARAM_INVALID)
    //{
    //    attenuation *= tex2D(pk_LightCookies, float3(lightuv, light.cookie_index)).r;
    //}

    if (light.shadowmap_index != LIGHT_PARAM_INVALID)
    {
        attenuation *= SampleLightShadowmap(light.shadowmap_index, lightuv, linearDistance);
    }
}

PKLight GetSurfaceLightDirect(uint index, in float3 worldpos, uint cascade)
{
    float3 posToLight, color;
    float attenuation;
    GetLight(index, worldpos, cascade, color, posToLight, attenuation);
    return PKLight(color * attenuation, posToLight);
}

PKLight GetSurfaceLight(uint index, in float3 worldpos, uint cascade)
{
    float3 posToLight, color;
    float attenuation;
    GetLight(PK_BUFFER_DATA(pk_GlobalLightsList, index), worldpos, cascade, color, posToLight, attenuation);
    return PKLight(color * attenuation, posToLight);
}

float3 GetVolumeLightColor(uint index, in float3 worldpos, float3 viewdir, uint cascade, float anisotropy)
{
    float3 posToLight, color;
    float attenuation;
    GetLight(PK_BUFFER_DATA(pk_GlobalLightsList, index), worldpos, cascade, color, posToLight, attenuation);
    return color * attenuation * GetLightAnisotropy(viewdir, posToLight, anisotropy);
}

LightTile GetLightTile(int3 coord)
{
    #if defined(PK_WRITE_CLUSTER_LIGHTS)
        return LightTile(0,0,0);
    #else
        return CreateLightTile(imageLoad(pk_LightTiles, coord).x);
    #endif
}

LightTile GetLightTile() 
{ 
    #if defined(SHADER_STAGE_FRAGMENT)
        return GetLightTile(GetTileIndexUV(gl_FragCoord.xy * pk_ScreenParams.zw, LinearizeDepth(gl_FragCoord.z))); 
    #else
        return LightTile(0,0,0);
    #endif
}

float4 FragmentPhysicallyBasedShading(SurfaceData surf, float3 viewdir, float3 worldpos)
{
    #if defined(SHADER_STAGE_FRAGMENT)
            // @TODO refactor this to be more generic :/
        #if defined(PK_META_DEPTH_NORMALS)
            return float4(WorldToViewDir(surf.normal), surf.roughness);
        #else
            surf.normal = normalize(surf.normal);
            surf.roughness = max(surf.roughness, 0.002);
    
            //Temp
            surf.albedo = 1.0f.xxx;
    
            // Fix edge artifacts for when normals are pointing away from camera.
            half shiftAmount = dot(surf.normal, viewdir);
            surf.normal = shiftAmount < 0.0f ? surf.normal + viewdir * (-shiftAmount + 1e-5f) : surf.normal;
    
            float3 specColor = lerp(pk_DielectricSpecular.rgb, surf.albedo, surf.metallic);
            float reflectivity = pk_DielectricSpecular.r + surf.metallic * pk_DielectricSpecular.a;
            surf.albedo *= 1.0f - reflectivity;
    
            #if defined(PK_TRANSPARENT_PBR)
                surf.albedo *= alpha;
                surf.alpha = reflectivity + surf.alpha * (1.0f - reflectivity);
            #endif
    
            INIT_BRDF_CACHE(surf.albedo, specColor, surf.normal, viewdir, reflectivity, surf.roughness);
            
            PKIndirect indirect;
            indirect.diffuse = SampleEnv(OctaUV(surf.normal), 1.0f);
            indirect.specular = SampleEnv(OctaUV(reflect(-viewdir, surf.normal)), surf.roughness);
    
            SampleScreenSpaceGI(indirect);
    
            float3 color = BRDF_PBS_DEFAULT_INDIRECT(indirect);
    
            color *= surf.occlusion;
    
            LightTile tile = GetLightTile();
    
            for (uint i = tile.start; i < tile.end; ++i)
            {
                PKLight light = GetSurfaceLight(i, worldpos, tile.cascade);
                color += BRDF_PBS_DEFAULT_DIRECT(light);
            }
    
            return float4(color, surf.alpha);
        #endif
    #else
        return 0.0.xxxx;
    #endif
}