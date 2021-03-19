#version 460

#Blend Off
#ZTest LEqual
#ZWrite On
#Cull Back

#multi_compile _ PK_NORMALMAPS
#multi_compile _ PK_HEIGHTMAPS
#multi_compile _ PK_ENABLE_INSTANCING

#include includes/Lighting.glsl

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

#pragma PROGRAM_VERTEX
layout(location = 0) in float3 in_POSITION0;
layout(location = 1) in float3 in_NORMAL;
layout(location = 2) in float4 in_TANGENT;
layout(location = 3) in float2 in_TEXCOORD0;

out FragmentVaryings varyings;

void main()
{
    varyings.vs_WORLDPOSITION = ObjectToWorldPos(in_POSITION0.xyz);
    varyings.vs_TEXCOORD0 = in_TEXCOORD0;
	gl_Position = WorldToClipPos(varyings.vs_WORLDPOSITION);

    #if defined(PK_NORMALMAPS) || defined(PK_HEIGHTMAPS)
        float3 tangent = ObjectToWorldDir(in_TANGENT.xyz);
        float3 bitangent = ObjectToWorldDir(in_TANGENT.w * cross(in_NORMAL.xyz, in_TANGENT.xyz));
        float3 normal = ObjectToWorldDir(in_NORMAL.xyz);
        float3x3 TSROTATION = float3x3(tangent, bitangent, normal);

        #if defined(PK_NORMALMAPS)
            varyings.vs_TSROTATION = TSROTATION;
        #endif

        #if defined(PK_HEIGHTMAPS)
            varyings.vs_TSVIEWDIRECTION = mul(inverse(TSROTATION), normalize(pk_WorldSpaceCameraPos.xyz - varyings.vs_WORLDPOSITION));
        #endif
    #endif

    #if !defined(PK_NORMALMAPS)
        varyings.vs_NORMAL = ObjectToWorldDir(in_NORMAL.xyz);
    #endif
};

#pragma PROGRAM_FRAGMENT
uniform sampler2D _AlbedoTexture;
uniform sampler2D _PBSTexture;
uniform sampler2D _NormalMap;
uniform sampler2D _HeightMap;
uniform float4 _Color = float4(1,1,1,1);
uniform float _Metallic = 1.0f;
uniform float _Roughness = 1.0f;
uniform float _Occlusion = 0.0f;
uniform float _NormalAmount = 0.5f;
uniform float _HeightAmount = 0.1f;

in FragmentVaryings varyings;
layout(location = 0) out float4 SV_Target0;

void main()
{
    float3 worldpos = varyings.vs_WORLDPOSITION;
    float3 viewdir = normalize(pk_WorldSpaceCameraPos.xyz - worldpos);
    float2 uv = varyings.vs_TEXCOORD0;

    float hh;

    #if defined(PK_HEIGHTMAPS)
        float heightval = tex2D(_HeightMap, uv).x;
        hh = heightval;
        uv.xy += ParallaxOffset(heightval, _HeightAmount, normalize(varyings.vs_TSVIEWDIRECTION));
    #endif

    SurfaceData surf; 
    surf.albedo = tex2D(_AlbedoTexture, uv).xyz;
    surf.alpha = 1.0f;
    surf.emission = float3(0,0,0);

    #if defined(PK_NORMALMAPS)
        surf.normal = SampleNormal(_NormalMap, varyings.vs_TSROTATION, uv, _NormalAmount);
    #else
        surf.normal = varyings.vs_NORMAL;
    #endif

    float3 pbsval = tex2D(_PBSTexture, uv).xyz;
    surf.metallic = pbsval.x * _Metallic;
    surf.roughness = pbsval.z * _Roughness;
    surf.occlusion = lerp(1.0f, pbsval.y, _Occlusion);
    
    SV_Target0 = PhysicallyBasedShading(surf, viewdir, worldpos);
};