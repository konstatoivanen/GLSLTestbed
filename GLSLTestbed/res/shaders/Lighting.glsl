#ifndef PK_Lighting
#define PK_Lighting

#include PKCommon.glsl

uniform sampler2D pk_SceneOEM_HDR;
uniform float4 pk_SceneOEM_ST;
uniform float pk_SceneOEM_RVS[3];

struct PKLight
{
    float3 color;
    float3 direction;
};

struct PKIndirect
{
    float3 diffuse;
    float3 specular;
};

struct PKGI
{
    PKLight light;
    PKIndirect indirect;
};

struct SurfaceData
{
    float3 albedo;      
    float3 normal;      
    float3 emission;
    float metallic;     
    float roughness;
    float occlusion;
    float alpha;
};

PKLight EmptyLight()
{
    return PKLight(float3(0,0,0), float3(0,1,0));
}

PKIndirect EmptyIndirect()
{
    return PKIndirect(float3(0,0,0), float3(0,0,0));
}

float2 OctaEncode(float3 n)
{
    float4 a;
    float2 b;
    bool3 c;

    a.x = abs(n.y) + abs(n.x);
    a.x = a.x + abs(n.z);
    a.xyz = n.yxz / a.xxx;
    b.xy = -abs(a.zy) + 1.0f;
    c.xyz = greaterThanEqual(a.xyz, float3(0.0, 0.0, 0.0)).xyz;
    a.x = (c.y) ? 1.0f : -1.0f;
    a.w = (c.z) ? 1.0f : -1.0f;
    a.xw = a.xw * b.xy;
    a.xy = (c.x) ? a.yz : a.xw;
    return a.xy * 0.5f + 0.5f;
}

float3 OctaDecode(float2 f)
{
    f = f * 2.0f - 1.0f;

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, 1.0f - abs(f.x) - abs(f.y), f.y);
    float t = max(-n.y, 0.0);
    n.x += n.x >= 0.0f ? -t : t;
    n.z += n.z >= 0.0f ? -t : t;
    return normalize(n);
}

float2 OctaUV(float3 reflection)
{
    float2 reflUV = OctaEncode(reflection);
    reflUV = pk_SceneOEM_ST.xy + reflUV * pk_SceneOEM_ST.z;
    return reflUV;
}

float2 OctaUV(float2 offset, float3 reflection)
{
    float2 reflUV = OctaEncode(reflection);
    reflUV = offset + reflUV * pk_SceneOEM_ST.z;
    return reflUV;
}

float3 SampleEnv(float2 uv, float roughness)
{
    float v0 = saturate((roughness - pk_SceneOEM_RVS[0]) / (pk_SceneOEM_RVS[1] - pk_SceneOEM_RVS[0]));
    float v1 = saturate((roughness - pk_SceneOEM_RVS[1]) / (pk_SceneOEM_RVS[2] - pk_SceneOEM_RVS[1]));
    float4 env = tex2DLod(pk_SceneOEM_HDR, uv, v0 + v1);
    return HDRDecode(env).rgb;
}


float3 FresnelTerm(float3 F0, float cosA)
{
    float t = pow5 (1 - cosA);
    return F0 + (1-F0).xxx * t;
}

float3 FresnelLerp(float3 F0, float3 F90, float cosA)
{
    float t = pow5(1 - cosA);
    return lerp(F0, F90, t);
}

float DisneyDiffuse(float  NdotV, float NdotL, float LdotH, float perceptualRoughness)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
    float lightScatter = (1 + (fd90 - 1) * pow5(1 - NdotL));
    float viewScatter = (1 + (fd90 - 1) * pow5(1 - NdotV));
    return lightScatter * viewScatter;
}

float GSF_SmithGGX(float NdotL, float NdotV, float roughness)
{
    float lambdaV = NdotL * (NdotV * (1 - roughness) + roughness);
    float lambdaL = NdotV * (NdotL * (1 - roughness) + roughness);
    return 0.5f / (lambdaV + lambdaL + 1e-5f);
}

float NDF_GGX(float NdotH, float roughness)
{
    float a2 = roughness * roughness;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0f;
    return PK_INV_PI * a2 / (d * d + 1e-7f);
}

float3 BRDF_PBS_DEFAULT(float3 diffuse, float3 specular, float oneMinusReflectivity, float roughness, float3 normal, float3 viewDir, const PKLight light, const PKIndirect gi)
{
    float perceptualRoughness = sqrt(roughness);

    float3 halfDir = normalize(light.direction + viewDir);
    float nv = max(0.0f, dot(normal, viewDir));
    float nl = max(0.0f, dot(normal, light.direction));
    float nh = max(0.0f, dot(normal, halfDir));
    float lv = max(0.0f, dot(light.direction, viewDir));
    float lh = max(0.0f, dot(light.direction, halfDir));

    float diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl * PK_INV_PI;
    
    float G = GSF_SmithGGX(nl, nv, roughness);
    float D = NDF_GGX(nh, roughness);
    
    float specularTerm = max(0, G * D * nl);
    
    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
    
    float grazingTerm = saturate((1 - perceptualRoughness) + (1 - oneMinusReflectivity));
    
    return diffuse * (gi.diffuse + light.color * diffuseTerm)
            + specularTerm * light.color * FresnelTerm(specular, lh)
            + surfaceReduction * gi.specular * FresnelLerp(specular, grazingTerm.xxx, nv);
}


float4 PhysicallyBasedShading(SurfaceData s, float3 viewDir)
{
    s.normal = normalize(s.normal);
    s.roughness = max(s.roughness, 0.002);

    // The amount we shift the normal toward the view vector is defined by the dot product.
    half shiftAmount = dot(s.normal, viewDir);
    s.normal = shiftAmount < 0.0f ? s.normal + viewDir * (-shiftAmount + 1e-5f) : s.normal;

    float3 specColor = lerp (pk_ColorSpaceDielectricSpec.rgb, s.albedo, s.metallic);
    float oneMinusReflectivity = pk_ColorSpaceDielectricSpec.a - s.metallic * pk_ColorSpaceDielectricSpec.a;
    s.albedo *= oneMinusReflectivity;

    // shader relies on pre-multiply alpha-blend (_SrcBlend = One, _DstBlend = OneMinusSrcAlpha)
    // this is necessary to handle transparency in physically correct way - only diffuse component gets affected by alpha
    #if defined(_ALPHAPREMULTIPLY_ON)
        s.albedo *= alpha;
        s.alpha = 1.0f - oneMinusReflectivity + s.alpha * oneMinusReflectivity;
    #endif

    PKGI gi;

    PKIndirect indirect;
    float2 reflUV = OctaUV(reflect(-viewDir, s.normal));
    indirect.diffuse = float3(0.0f, 0.05f, 0.125f);
    indirect.specular = SampleEnv(reflUV, s.roughness);

    PKLight light;
    light.color = float3(1.0f,0.9f,0.5f) * 2.5f;
    light.direction = float3(0,1,0);

    gi.light = light;
    gi.indirect = EmptyIndirect();

    float3 c = BRDF_PBS_DEFAULT(s.albedo, specColor, oneMinusReflectivity, s.roughness, s.normal, viewDir, gi.light, gi.indirect);

    gi.light.direction = float3(1, 0, 0);
    gi.light.color = float3(0.5f, 1.5f, 2.0f);

    c += BRDF_PBS_DEFAULT(s.albedo, specColor, oneMinusReflectivity, s.roughness, s.normal, viewDir, gi.light, gi.indirect);

    gi.light = EmptyLight(); 
    gi.indirect = indirect;

    c += BRDF_PBS_DEFAULT(s.albedo, specColor, oneMinusReflectivity, s.roughness, s.normal, viewDir, gi.light, gi.indirect);

    return float4(c, s.alpha);
}
#endif