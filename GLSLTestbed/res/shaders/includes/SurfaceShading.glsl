#pragma once
#include Lighting.glsl
#include Reconstruction.glsl
#if defined(PK_META_GI_VOXELIZE)
#include SceneGIShared.glsl
#endif

#define SRC_METALLIC x
#define SRC_OCCLUSION y
#define SRC_ROUGHNESS z

#if defined(PK_META_GI_VOXELIZE)
    #define PK_INIT_CLIP_UV(w, c)           \
        float3 c;                           \
        if (!TryGetWorldToClipUVW(w, c))    \
        {                                   \
            return;                         \
        }                                   \

#else
    #define PK_INIT_CLIP_UV(w, c) float3 c = GetFragmentClipUVW(); 

#endif

float3 GetSurfaceSpecularColor(float3 albedo, float metallic) { return lerp(pk_DielectricSpecular.rgb, albedo, metallic); }

float GetSurfaceAlphaReflectivity(inout SurfaceData surf)
{
    float reflectivity = pk_DielectricSpecular.r + surf.metallic * pk_DielectricSpecular.a;
    surf.albedo *= 1.0f - reflectivity;
    
    #if defined(PK_TRANSPARENT_PBR)
        surf.albedo *= alpha;
        surf.alpha = reflectivity + surf.alpha * (1.0f - reflectivity);
    #endif

    return reflectivity;
}

float4 FragmentPhysicallyBasedShading(SurfaceData surf, float3 viewdir, float3 worldpos, float3 clipuvw)
{
    #if defined(SHADER_STAGE_FRAGMENT)
            // @TODO refactor this to be more generic :/
        #if defined(PK_META_DEPTH_NORMALS)

            return float4(WorldToViewDir(surf.normal), surf.roughness);
        #elif defined(PK_META_GI_VOXELIZE)

            float3 specColor = GetSurfaceSpecularColor(surf.albedo, surf.metallic);
            float reflectivity = GetSurfaceAlphaReflectivity(surf);
    
            float3 color = 0.0f.xxx;
    
            LightTile tile = GetLightTile(clipuvw);
    
            for (uint i = tile.start; i < tile.end; ++i)
            {
                PKLight light = GetSurfaceLight(i, worldpos, tile.cascade);
                color += surf.albedo * light.color * max(0.0f, dot(light.direction, surf.normal));
            }
    
            color.rgb += surf.emission;

            return float4(color, surf.alpha); 
        #else

            surf.roughness = max(surf.roughness, 0.002);
    
            // Fix edge artifacts for when normals are pointing away from camera.
            half shiftAmount = dot(surf.normal, viewdir);
            surf.normal = shiftAmount < 0.0f ? surf.normal + viewdir * (-shiftAmount + 1e-5f) : surf.normal;
    
            float3 specColor = GetSurfaceSpecularColor(surf.albedo, surf.metallic);
            float reflectivity = GetSurfaceAlphaReflectivity(surf);
    
            INIT_BRDF_CACHE(surf.albedo, specColor, surf.normal, viewdir, reflectivity, surf.roughness);
            
            PKIndirect indirect;
            indirect.diffuse = SampleEnv(OctaUV(surf.normal), 1.0f);
            indirect.specular = SampleEnv(OctaUV(reflect(-viewdir, surf.normal)), surf.roughness);
    
            SampleScreenSpaceGI(indirect, clipuvw.xy);
    
            float3 color = BRDF_PBS_DEFAULT_INDIRECT(indirect);
    
            color *= surf.occlusion;
    
            LightTile tile = GetLightTile(clipuvw);
    
            for (uint i = tile.start; i < tile.end; ++i)
            {
                PKLight light = GetSurfaceLight(i, worldpos, tile.cascade);
                color += BRDF_PBS_DEFAULT_DIRECT(light);
            }
    
            color.rgb += surf.emission;

            return float4(color, surf.alpha);
        #endif
    #else
        return 0.0.xxxx;
    #endif
}