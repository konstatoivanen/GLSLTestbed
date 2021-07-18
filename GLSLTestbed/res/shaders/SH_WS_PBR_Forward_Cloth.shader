#version 460
#extension GL_ARB_bindless_texture : require

#Blend Off
#ZTest Equal
#ZWrite Off
#Cull Back

#multi_compile _ PK_NORMALMAPS
#multi_compile _ PK_ENABLE_INSTANCING

#define PK_ACTIVE_BRDF BRDF_PBS_CLOTH_DIRECT
#define PK_ACTIVE_VXGI_BRDF BRDF_VXGI_CLOTH

#include includes/SurfaceShading.glsl

PK_BEGIN_INSTANCED_PROPERTIES
	PK_INSTANCED_PROPERTY float4 _Color;
	PK_INSTANCED_PROPERTY float4 _SheenColor;
	PK_INSTANCED_PROPERTY sampler2D _AlbedoTexture;
	PK_INSTANCED_PROPERTY sampler2D _PBSTexture;
	PK_INSTANCED_PROPERTY sampler2D _NormalMap;
	PK_INSTANCED_PROPERTY sampler2D _HeightMap;
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

    surf.subsurface_distortion = 0.1f;
    surf.subsurface_power = 2.0f;
    surf.subsurface_thickness = 0.4f;
    surf.sheen = PK_ACCESS_INSTANCED_PROP(_SheenColor).rgb;
    surf.normal = PK_SURF_SAMPLE_NORMAL(_NormalMap, _NormalAmount, uv);

    float3 textureval = tex2D(PK_ACCESS_INSTANCED_PROP(_PBSTexture), uv).xyz;
    surf.roughness = textureval.SRC_ROUGHNESS * PK_ACCESS_INSTANCED_PROP(_Roughness);
    surf.occlusion = lerp(1.0f, textureval.SRC_OCCLUSION, PK_ACCESS_INSTANCED_PROP(_Occlusion));
    surf.occlusion = SampleScreenSpaceOcclusion(surf.occlusion, surf.clipuvw.xy);
};