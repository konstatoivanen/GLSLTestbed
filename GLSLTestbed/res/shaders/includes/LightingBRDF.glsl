#pragma once
#include HLSLSupport.glsl
#include LightingCommon.glsl

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

    float3 halfDir = normalize(light.direction.xyz + viewDir);
    float nv = max(0.0f, dot(normal, viewDir));
    float nl = max(0.0f, dot(normal, light.direction.xyz));
    float nh = max(0.0f, dot(normal, halfDir));
    float lv = max(0.0f, dot(light.direction.xyz, viewDir));
    float lh = max(0.0f, dot(light.direction.xyz, halfDir));

    float diffuseTerm = DisneyDiffuse(nv, nl, lh, perceptualRoughness) * nl * PK_INV_PI;
    
    float G = GSF_SmithGGX(nl, nv, roughness);
    float D = NDF_GGX(nh, roughness);
    
    float specularTerm = max(0, G * D * nl);
    
    float surfaceReduction = 1.0 / (roughness * roughness + 1.0);
    
    float grazingTerm = saturate((1 - perceptualRoughness) + (1 - oneMinusReflectivity));
    
    return diffuse * (gi.diffuse.rgb + light.color.rgb * diffuseTerm)
            + specularTerm * light.color.rgb * FresnelTerm(specular, lh)
            + surfaceReduction * gi.specular.rgb * FresnelLerp(specular, grazingTerm.xxx, nv);
}