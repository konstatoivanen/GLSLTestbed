#version 460
#extension GL_ARB_bindless_texture : require

#Blend Off
#ZTest Equal
#ZWrite Off
#Cull Back

#multi_compile _ PK_NORMALMAPS
#multi_compile _ PK_HEIGHTMAPS
#multi_compile _ PK_EMISSION
#multi_compile _ PK_ENABLE_INSTANCING

#include includes/SurfaceShading.glsl

PK_BEGIN_INSTANCED_PROPERTIES
    PK_INSTANCED_PROPERTY float4 _Color;
    PK_INSTANCED_PROPERTY float4 _EmissionColor;
    PK_INSTANCED_PROPERTY sampler2D _AlbedoTexture;
    PK_INSTANCED_PROPERTY sampler2D _PBSTexture;
    PK_INSTANCED_PROPERTY sampler2D _NormalMap;
    PK_INSTANCED_PROPERTY sampler2D _HeightMap;
    PK_INSTANCED_PROPERTY sampler2D _EmissionTexture;
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

    uv.xy += PK_SURF_SAMPLE_PARALLAX_OFFSET(_HeightMap, _HeightAmount);

    surf.albedo = tex2D(PK_ACCESS_INSTANCED_PROP(_AlbedoTexture), uv).xyz * PK_ACCESS_INSTANCED_PROP(_Color).xyz;
    surf.alpha = 1.0f;

    #if defined(PK_EMISSION)
        // GI color test code
        float lval = surf.worldpos.z * 0.025f + pk_Time.y * 0.25f;
        lval -= floor(lval);
        lval = 1.0f - lval;
        lval -= 0.75f;
        lval *= 4.0f;
        lval = saturate(lval);
        lval *= pow5(lval);
        float3 c = float3(surf.worldpos.z * 0.025f + pk_Time.y, 1.0f, lval * 20.0f);
        
        float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        float3 ecolor = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
         
        surf.emission = tex2D(PK_ACCESS_INSTANCED_PROP(_EmissionTexture), uv * 8.0f).rgb * ecolor;//PK_ACCESS_INSTANCED_PROP(_EmissionColor).rgb;
    #endif

    surf.normal = PK_SURF_SAMPLE_NORMAL(_NormalMap, _NormalAmount, uv);

    float3 textureval = tex2D(PK_ACCESS_INSTANCED_PROP(_PBSTexture), uv).xyz;
    surf.metallic = textureval.SRC_METALLIC * PK_ACCESS_INSTANCED_PROP(_Metallic);
    surf.roughness = textureval.SRC_ROUGHNESS * PK_ACCESS_INSTANCED_PROP(_Roughness);
    surf.occlusion = lerp(1.0f, textureval.SRC_OCCLUSION, PK_ACCESS_INSTANCED_PROP(_Occlusion));
    surf.occlusion = SampleScreenSpaceOcclusion(surf.occlusion, surf.clipuvw.xy);
}