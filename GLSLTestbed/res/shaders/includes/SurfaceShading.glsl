#pragma once
#include Lighting.glsl
#include LightingBRDF.glsl
#include Reconstruction.glsl
#include SceneGIShared.glsl

// Meta pass specific parameters (gi voxelization requires some changes from reqular view projection).
#multi_compile _ PK_META_DEPTH_NORMALS PK_META_GI_VOXELIZE

#if defined(PK_META_GI_VOXELIZE)
    #undef PK_NORMALMAPS
    #undef PK_HEIGHTMAPS
    
    #include SceneGIShared.glsl

    #define PK_META_EARLY_CLIP_UVW(w, c)    \
        if (!TryGetWorldToClipUVW(w, c))    \
        {                                   \
            return;                         \
        }                                   \

    #define PK_META_DECLARE_SURFACE_OUTPUT

    #define PK_META_STORE_SURFACE_OUTPUT(color, worldpos) StoreSceneGI(worldpos, color)

    #define PK_META_WORLD_TO_CLIPSPACE(position)  WorldToVoxelNDCSpace(position)

    float SampleScreenSpaceOcclusion(float occlusion, float2 uv) { return occlusion; }

#else
    #define PK_META_EARLY_CLIP_UVW(w, c) c = GetFragmentClipUVW(); 

    #define PK_META_DECLARE_SURFACE_OUTPUT layout(location = 0) out float4 SV_Target0;

    #define PK_META_STORE_SURFACE_OUTPUT(color, worldpos) SV_Target0 = color

    #define PK_META_WORLD_TO_CLIPSPACE(position) WorldToClipPos(position)

    float SampleScreenSpaceOcclusion(float occlusion, float2 uv) { return min(occlusion, SampleScreenSpaceOcclusion(uv)); }

#endif

#define SRC_METALLIC x
#define SRC_OCCLUSION y
#define SRC_ROUGHNESS z

struct SurfaceFragmentVaryings
{
    float2 vs_TEXCOORD0;
    float3 vs_WORLDPOSITION;
    #if defined(PK_NORMALMAPS)
        float3x3 vs_TSROTATION;
    #else
        float3 vs_NORMAL;
    #endif
    #if defined(PK_HEIGHTMAPS)
        float3 vs_TSVIEWDIRECTION;
    #endif
};


struct SurfaceData
{
    float3 viewdir;
    float3 worldpos;
    float3 clipuvw;

    float3 albedo;      
    float3 normal;      
    float3 emission;
    float metallic;     
    float roughness;
    float occlusion;
    float thickness;
    float alpha;
};

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

#if defined(SHADER_STAGE_VERTEX)

    // Use these to modify surface values in fragment or vertex stage
    void PK_SURFACE_FUNC_VERT(inout SurfaceFragmentVaryings surf);

    layout(location = 0) in float3 in_POSITION0;
    layout(location = 1) in float3 in_NORMAL;
    layout(location = 2) in float4 in_TANGENT;
    layout(location = 3) in float2 in_TEXCOORD0;
    
    out SurfaceFragmentVaryings baseVaryings;
    PK_VARYING_INSTANCE_ID
    
    void main()
    {
        baseVaryings.vs_WORLDPOSITION = ObjectToWorldPos(in_POSITION0.xyz);
        baseVaryings.vs_TEXCOORD0 = in_TEXCOORD0;
        gl_Position = PK_META_WORLD_TO_CLIPSPACE(baseVaryings.vs_WORLDPOSITION);
    
        PK_SETUP_INSTANCE_ID();
    
        #if defined(PK_NORMALMAPS) || defined(PK_HEIGHTMAPS)
            float3x3 TBN = ComposeMikkTangentSpaceMatrix(in_NORMAL, in_TANGENT);
    
            #if defined(PK_NORMALMAPS)
                baseVaryings.vs_TSROTATION = TBN;
            #endif
    
            #if defined(PK_HEIGHTMAPS)
                baseVaryings.vs_TSVIEWDIRECTION = mul(transpose(TBN), normalize(pk_WorldSpaceCameraPos.xyz - baseVaryings.vs_WORLDPOSITION));
            #endif
        #endif
    
        #if !defined(PK_NORMALMAPS)
            baseVaryings.vs_NORMAL = ObjectToWorldDir(in_NORMAL.xyz);
        #endif

        PK_SURFACE_FUNC_VERT(baseVaryings);
    };

#elif defined(SHADER_STAGE_FRAGMENT)

    // Use these to modify surface values in fragment or vertex stage
    void PK_SURFACE_FUNC_FRAG(in SurfaceFragmentVaryings varyings, inout SurfaceData surf);

    in SurfaceFragmentVaryings baseVaryings;
    PK_VARYING_INSTANCE_ID
    PK_META_DECLARE_SURFACE_OUTPUT
    
    #if defined(PK_HEIGHTMAPS)
        #define PK_SURF_SAMPLE_PARALLAX_OFFSET(heightmap, amount) ParallaxOffset(tex2D(PK_ACCESS_INSTANCED_PROP(heightmap), uv).x, PK_ACCESS_INSTANCED_PROP(amount), normalize(baseVaryings.vs_TSVIEWDIRECTION));

    #else
        #define PK_SURF_SAMPLE_PARALLAX_OFFSET(heightmap, amount) 0.0f.xx 
    
    #endif

    #if defined(PK_NORMALMAPS)
         #define PK_SURF_SAMPLE_NORMAL(normalmap, amount, uv) SampleNormal(PK_ACCESS_INSTANCED_PROP(normalmap), baseVaryings.vs_TSROTATION, uv, PK_ACCESS_INSTANCED_PROP(amount))
    #else
         #define PK_SURF_SAMPLE_NORMAL(normalmap, amount, uv) varyings.vs_NORMAL
    #endif

    void main()
    {
        PK_SETUP_INSTANCE_ID();
    
        SurfaceData surf; 
        surf.worldpos = baseVaryings.vs_WORLDPOSITION;
        surf.viewdir = normalize(pk_WorldSpaceCameraPos.xyz - surf.worldpos);
    
        PK_META_EARLY_CLIP_UVW(surf.worldpos, surf.clipuvw)
    
        PK_SURFACE_FUNC_FRAG(baseVaryings, surf);

        float4 outvalue;

        // @TODO refactor this to be more generic
        #if defined(PK_META_DEPTH_NORMALS)

            outvalue = float4(WorldToViewDir(surf.normal), surf.roughness);
        #elif defined(PK_META_GI_VOXELIZE)

            float3 specColor = GetSurfaceSpecularColor(surf.albedo, surf.metallic);
            float reflectivity = GetSurfaceAlphaReflectivity(surf);
    
            float3 color = 0.0f.xxx;
    
            LightTile tile = GetLightTile(surf.clipuvw);
    
            for (uint i = tile.start; i < tile.end; ++i)
            {
                PKLight light = GetSurfaceLight(i, surf.worldpos, tile.cascade);
                color += surf.albedo * light.color * max(0.0f, dot(light.direction, surf.normal));
            }
    
            color.rgb += surf.emission;

            // Multi bounce gi. Causes some very lingering light artifacts & bleeding. @TODO Consider adding a setting for this.
            color.rgb += surf.albedo * ConeTraceDiffuse(surf.worldpos, surf.normal, 0.0f).rgb * 0.95f;

            outvalue = float4(color, surf.alpha); 
        #else

            surf.roughness = max(surf.roughness, 0.002);
    
            // Fix edge artifacts for when normals are pointing away from camera.
            half shiftAmount = dot(surf.normal, surf.viewdir);
            surf.normal = shiftAmount < 0.0f ? surf.normal + surf.viewdir * (-shiftAmount + 1e-5f) : surf.normal;
    
            float3 specColor = GetSurfaceSpecularColor(surf.albedo, surf.metallic);
            float reflectivity = GetSurfaceAlphaReflectivity(surf);
    
            INIT_BRDF_CACHE(surf.albedo, specColor, surf.normal, surf.viewdir, reflectivity, surf.roughness, surf.thickness);
            
            PKIndirect indirect;
            indirect.diffuse = SampleEnvironment(OctaUV(surf.normal), 1.0f);
            indirect.specular = SampleEnvironment(OctaUV(reflect(-surf.viewdir, surf.normal)), surf.roughness);
    
            SampleScreenSpaceGI(indirect, surf.clipuvw.xy);
    
            float3 color = BRDF_PBS_DEFAULT_INDIRECT(indirect);
    
            color *= surf.occlusion;
    
            LightTile tile = GetLightTile(surf.clipuvw);
    
            for (uint i = tile.start; i < tile.end; ++i)
            {
                PKLight light = GetSurfaceLight(i, surf.worldpos, tile.cascade);
                color += BRDF_PBS_DEFAULT_DIRECT(light);
            }
    
            color.rgb += surf.emission;

            outvalue = float4(color, surf.alpha);
        #endif

        PK_META_STORE_SURFACE_OUTPUT(outvalue, surf.worldpos);
    };

#endif