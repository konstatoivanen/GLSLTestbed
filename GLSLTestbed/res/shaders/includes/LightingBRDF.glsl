#pragma once
#include HLSLSupport.glsl
#include LightingCommon.glsl

struct BRDF_DATA_CACHE
{
    float3 diffuse;
    float3 specular;
    float3 normal;
    float3 viewdir;
    float reflectivity;
    float roughness;
    float perceptualRoughness;
    float grazingTerm;
    float surfaceReduction;
    float thickness;
    float nv;
};

BRDF_DATA_CACHE brdf_cache;


float3 FresnelTerm(float3 F0, float cosA)
{
    float t = pow5(1 - cosA);
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

float NDF_Charlie(float NdotH, float roughness) 
{
    float invAlpha  = 1.0 / roughness;
    float sin2h = max(1.0 - pow2(NdotH), 0.0078125);
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PK_PI);
}

// Source: https://www.gdcvault.com/play/1014538/Approximating-Translucency-for-a-Fast
float TDF_Dice(float3 viewdir, float3 lightdir, float3 normal, float distortion, float power, float thickness)
{
    return pow(saturate(dot(viewdir, -(lightdir + normal * distortion))), power) * thickness;
}

float3 BRDF_PBS_DEFAULT(float3 diffuse, float3 specular, float reflectivity, float roughness, float3 normal, float3 viewDir, const PKLight light, const PKIndirect gi)
{
    float perceptualRoughness = sqrt(roughness);

    float3 halfDir = normalize(light.direction.xyz + viewDir);
    float nv = max(0.0f, dot(normal, viewDir));
    float nl = max(0.0f, dot(normal, light.direction.xyz));
    float nh = max(0.0f, dot(normal, halfDir));
    float lh = max(0.0f, dot(light.direction.xyz, halfDir));

    float diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl * PK_INV_PI;
    
    float G = GSF_SmithGGX(nl, nv, roughness);
    float D = NDF_GGX(nh, roughness);
    
    float specularTerm = max(0, G * D * nl);
    
    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
    
    float grazingTerm = saturate((1 - perceptualRoughness) + reflectivity);
    
    return diffuse * (gi.diffuse.rgb + light.color.rgb * diffuseTerm)
            + specularTerm * light.color.rgb * FresnelTerm(specular, lh)
            + surfaceReduction * gi.specular.rgb * FresnelLerp(specular, grazingTerm.xxx, nv);
}

void INIT_BRDF_CACHE(float3 diffuse, float3 specular, float3 normal, float3 viewdir, float reflectivity, float roughness, float thickness)
{
    brdf_cache.diffuse = diffuse;
    brdf_cache.specular = specular;
    brdf_cache.normal = normal;
    brdf_cache.viewdir = viewdir;
    brdf_cache.reflectivity = reflectivity;
    brdf_cache.roughness = roughness;
    brdf_cache.perceptualRoughness = sqrt(roughness);
    brdf_cache.thickness = thickness;
    brdf_cache.grazingTerm = saturate((1 - brdf_cache.perceptualRoughness) + reflectivity);
    brdf_cache.surfaceReduction = 1.0 / (roughness * roughness + 1.0);
    brdf_cache.nv = max(0.0f, dot(normal, viewdir));
}

float3 BRDF_PBS_DEFAULT_INDIRECT(const PKIndirect gi)
{
    return brdf_cache.diffuse * gi.diffuse.rgb + brdf_cache.surfaceReduction * gi.specular.rgb * FresnelLerp(brdf_cache.specular, brdf_cache.grazingTerm.xxx, brdf_cache.nv);
}

float3 BRDF_PBS_DEFAULT_DIRECT(const PKLight light)
{
    float3 halfDir = normalize(light.direction.xyz + brdf_cache.viewdir);
    float3 ldots;
    ldots.x = dot(brdf_cache.normal, light.direction.xyz); // NL
    ldots.y = dot(brdf_cache.normal, halfDir); // NH
    ldots.z = dot(light.direction.xyz, halfDir); // LH
    ldots = max(0.0f.xxx, ldots);

    float diffuseTerm = DisneyDiffuse(brdf_cache.nv, ldots.x, ldots.z, brdf_cache.perceptualRoughness) * ldots.x * PK_INV_PI;
    float G = GSF_SmithGGX(ldots.x, brdf_cache.nv, brdf_cache.roughness);
    float D = NDF_GGX(ldots.y, brdf_cache.roughness);
    
    float specularTerm = max(0.0f, G * D * ldots.x);
    
    return brdf_cache.diffuse * light.color.rgb * diffuseTerm + specularTerm * light.color.rgb * FresnelTerm(brdf_cache.specular, ldots.z);
}

float3 BRDF_PBS_SSS_DIRECT(const PKLight light)
{
    float3 halfDir = normalize(light.direction.xyz + brdf_cache.viewdir);
    float3 ldots;
    ldots.x = dot(brdf_cache.normal, light.direction.xyz); // NL
    ldots.y = dot(brdf_cache.normal, halfDir); // NH
    ldots.z = dot(light.direction.xyz, halfDir); // LH
    ldots = max(0.0f.xxx, ldots);

    float sssterm = TDF_Dice(brdf_cache.viewdir, light.direction.xyz, brdf_cache.normal, 0.4f, 4.0f, brdf_cache.thickness);
    float diffuseTerm = DisneyDiffuse(brdf_cache.nv, ldots.x, ldots.z, brdf_cache.perceptualRoughness) * ldots.x * PK_INV_PI;
    float G = GSF_SmithGGX(ldots.x, brdf_cache.nv, brdf_cache.roughness);
    float D = NDF_GGX(ldots.y, brdf_cache.roughness);
    
    float specularTerm = max(0.0f, G * D * ldots.x);
    
    return brdf_cache.diffuse * light.color.rgb * diffuseTerm + 
           brdf_cache.diffuse * light.color.rgb * sssterm +
           specularTerm * light.color.rgb * FresnelTerm(brdf_cache.specular, ldots.z);
}