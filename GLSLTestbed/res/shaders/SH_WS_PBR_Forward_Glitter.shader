#version 460
#extension GL_ARB_bindless_texture : require

#Blend Off
#ZTest Equal
#ZWrite Off
#Cull Back

#multi_compile _ PK_NORMALMAPS
#multi_compile _ PK_HEIGHTMAPS
#multi_compile _ PK_ENABLE_INSTANCING

#include includes/SurfaceShading.glsl

PK_BEGIN_INSTANCED_PROPERTIES
    PK_INSTANCED_PROPERTY float4 _Color;
    PK_INSTANCED_PROPERTY sampler2D _AlbedoTexture;
    PK_INSTANCED_PROPERTY sampler2D _PBSTexture;
    PK_INSTANCED_PROPERTY sampler2D _NormalMap;
    PK_INSTANCED_PROPERTY sampler2D _HeightMap;
    PK_INSTANCED_PROPERTY float _Metallic;
    PK_INSTANCED_PROPERTY float _Roughness;
    PK_INSTANCED_PROPERTY float _Occlusion;
    PK_INSTANCED_PROPERTY float _NormalAmount;
    PK_INSTANCED_PROPERTY float _HeightAmount;
PK_END_INSTANCED_PROPERTIES

#pragma PROGRAM_VERTEX
void PK_SURFACE_FUNC_VERT(inout SurfaceFragmentVaryings surf) {}

#pragma PROGRAM_FRAGMENT

void PK_SURFACE_FUNC_FRAG(in SurfaceFragmentVaryings varyings, inout SurfaceData surf)
{
    float2 uv = varyings.vs_TEXCOORD0;

    #if defined(PK_HEIGHTMAPS)
        float heightval = tex2D(PK_ACCESS_INSTANCED_PROP(_HeightMap), uv).x;
        uv.xy += ParallaxOffset(heightval, PK_ACCESS_INSTANCED_PROP(_HeightAmount), normalize(varyings.vs_TSVIEWDIRECTION));
    #endif

    surf.albedo = tex2D(PK_ACCESS_INSTANCED_PROP(_AlbedoTexture), uv).xyz * PK_ACCESS_INSTANCED_PROP(_Color).xyz;
    surf.alpha = 1.0f;

    #if defined(PK_NORMALMAPS)
        surf.normal = SampleNormal(PK_ACCESS_INSTANCED_PROP(_NormalMap), varyings.vs_TSROTATION, uv, PK_ACCESS_INSTANCED_PROP(_NormalAmount));
    #else
        surf.normal = varyings.vs_NORMAL;
    #endif

    float3 textureval = tex2D(PK_ACCESS_INSTANCED_PROP(_PBSTexture), uv).xyz;
    surf.metallic = textureval.SRC_METALLIC * PK_ACCESS_INSTANCED_PROP(_Metallic);
    surf.roughness = textureval.SRC_ROUGHNESS * PK_ACCESS_INSTANCED_PROP(_Roughness);
    surf.occlusion = lerp(1.0f, textureval.SRC_OCCLUSION, PK_ACCESS_INSTANCED_PROP(_Occlusion));
    surf.occlusion = SampleScreenSpaceOcclusion(surf.occlusion, surf.clipuvw.xy);

    // Add glitter
    float t = tex2D(pk_Bluenoise256, uv * 2).r;
    t = pow5(t);
    t -= 0.75f;
    t *= 4.0f;
    t = max(0.0f, t);
    surf.roughness -= t;
};