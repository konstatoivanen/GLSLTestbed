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
#multi_compile _ PK_META_DEPTH_NORMALS PK_META_GI_VOXELIZE

#if defined(PK_META_GI_VOXELIZE)
    #undef PK_NORMALMAPS
    #undef PK_HEIGHTMAPS
#endif

#include includes/SurfaceShading.glsl

struct FragmentVaryings
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
layout(location = 0) in float3 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;
layout(location = 2) in float4 in_TANGENT;
layout(location = 3) in float2 in_TEXCOORD0;

out FragmentVaryings varyings;
PK_VARYING_INSTANCE_ID

void main()
{
    varyings.vs_WORLDPOSITION = ObjectToWorldPos(in_POSITION0.xyz);
    varyings.vs_TEXCOORD0 = in_TEXCOORD0;

    #if defined(PK_META_GI_VOXELIZE)
        gl_Position = WorldToVoxelNDCSpace(varyings.vs_WORLDPOSITION);
    #else 
        gl_Position = WorldToClipPos(varyings.vs_WORLDPOSITION);
    #endif

    PK_SETUP_INSTANCE_ID();

    #if defined(PK_NORMALMAPS) || defined(PK_HEIGHTMAPS)
        float3x3 TBN = ComposeMikkTangentSpaceMatrix(in_NORMAL, in_TANGENT);

        #if defined(PK_NORMALMAPS)
            varyings.vs_TSROTATION = TBN;
        #endif

        #if defined(PK_HEIGHTMAPS)
            varyings.vs_TSVIEWDIRECTION = mul(transpose(TBN), normalize(pk_WorldSpaceCameraPos.xyz - varyings.vs_WORLDPOSITION));
        #endif
    #endif

    #if !defined(PK_NORMALMAPS)
        varyings.vs_NORMAL = ObjectToWorldDir(in_NORMAL.xyz);
    #endif
};

#pragma PROGRAM_FRAGMENT

float3 hsv2rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

in FragmentVaryings varyings;
PK_VARYING_INSTANCE_ID

#if !defined(PK_META_GI_VOXELIZE)
    layout(location = 0) out float4 SV_Target0;
#endif

void main()
{
    PK_SETUP_INSTANCE_ID();

    float3 worldpos = varyings.vs_WORLDPOSITION;
    float3 viewdir = normalize(pk_WorldSpaceCameraPos.xyz - worldpos);
    float2 uv = varyings.vs_TEXCOORD0;

    PK_INIT_CLIP_UV(worldpos, clipuvw)

    #if defined(PK_HEIGHTMAPS)
        float heightval = tex2D(PK_ACCESS_INSTANCED_PROP(_HeightMap), uv).x;
        uv.xy += ParallaxOffset(heightval, PK_ACCESS_INSTANCED_PROP(_HeightAmount), normalize(varyings.vs_TSVIEWDIRECTION));
    #endif

    SurfaceData surf; 
    surf.albedo = tex2D(PK_ACCESS_INSTANCED_PROP(_AlbedoTexture), uv).xyz * PK_ACCESS_INSTANCED_PROP(_Color).xyz;
    surf.alpha = PK_ACCESS_INSTANCED_PROP(_Color).a;

    #if defined(PK_EMISSION)
       // GI color test code
       // float lval = worldpos.z * 0.025f + pk_Time.w * 0.25f;
       // lval -= floor(lval);
       // lval *= pow4(lval);
       // float3 ecolor = hsv2rgb(float3(worldpos.z * 0.025f + pk_Time.y, 1.0f, lval * 20.0f));

        surf.emission = tex2D(PK_ACCESS_INSTANCED_PROP(_EmissionTexture), uv).rgb * PK_ACCESS_INSTANCED_PROP(_EmissionColor).rgb;
    #else
        surf.emission = float3(0,0,0);
    #endif

    #if defined(PK_NORMALMAPS)
        surf.normal = SampleNormal(PK_ACCESS_INSTANCED_PROP(_NormalMap), varyings.vs_TSROTATION, uv, PK_ACCESS_INSTANCED_PROP(_NormalAmount));
    #else
        surf.normal = varyings.vs_NORMAL;
    #endif

    float3 textureval = tex2D(PK_ACCESS_INSTANCED_PROP(_PBSTexture), uv).xyz;
    surf.metallic = textureval.SRC_METALLIC * PK_ACCESS_INSTANCED_PROP(_Metallic);
    surf.roughness = textureval.SRC_ROUGHNESS * PK_ACCESS_INSTANCED_PROP(_Roughness);

    #if !defined(PK_META_GI_VOXELIZE)
        surf.occlusion = lerp(1.0f, textureval.SRC_OCCLUSION, PK_ACCESS_INSTANCED_PROP(_Occlusion)) * SampleScreenSpaceOcclusion(clipuvw.xy);
    #endif

    float4 color = FragmentPhysicallyBasedShading(surf, viewdir, worldpos, clipuvw);

    #if defined(PK_META_GI_VOXELIZE)
        StoreSceneGI(worldpos, color);
    #else
        SV_Target0 = color;
    #endif
};