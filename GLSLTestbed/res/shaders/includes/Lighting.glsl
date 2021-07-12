#pragma once
#include PKCommon.glsl
#include ClusterIndexing.glsl
#include LightingCommon.glsl
#include Encoding.glsl

#define SHADOW_USE_LBR 
#define SHADOW_LBR 0.2f
#define SHADOWMAP_CASCADES 4

//----------MATH UTILITIES----------//

#if defined(SHADOW_USE_LBR)
    float LBR(float shadow) { return smoothstep(SHADOW_LBR, 1.0f, shadow);}
#else
    #define LBR(shadow) (shadow)
#endif


// Source: https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
float GetAttenuation(float ldist, float lradius) { return pow2(saturate(1.0f - pow4(ldist/lradius))) / (pow2(ldist) + 1.0f); }

float GetLightAnisotropy(float3 viewdir, float3 posToLight, float anistropy)
{
	float gsq = pow2(anistropy);
	float denom = 1.0 + gsq - 2.0 * anistropy * dot(viewdir, posToLight);
	return (1.0 - gsq) * inversesqrt(max(0, pow3(denom)));
}


//----------INDIRECT SAMPLERS----------//
float3 SampleEnvironment(float2 uv, float roughness) { return HDRDecode(tex2DLod(pk_SceneOEM_HDR, uv, roughness * 4)) * pk_SceneOEM_Exposure; }

float SampleScreenSpaceOcclusion(float2 uv) { return tex2D(pk_ScreenOcclusion, uv).r; }

void SampleScreenSpaceGI(inout PKIndirect indirect, float2 uv)
{
    float4 diffuse = tex2D(pk_ScreenGI_Diffuse, uv);
    float4 specular = tex2D(pk_ScreenGI_Specular, uv);
    indirect.diffuse = indirect.diffuse * diffuse.a + diffuse.rgb;
    indirect.specular = indirect.specular * specular.a + specular.rgb;
}

float SampleLightShadowmap(uint shadowmapIndex, float2 uv, float lightDistance)
{
    float2 moments = tex2D(pk_ShadowmapArray, float3(uv, shadowmapIndex)).xy;
    float variance = moments.y - moments.x * moments.x;
    float difference = lightDistance - moments.x;
    return difference > 0.01f ? LBR(variance / (variance + difference * difference)) : 1.0f;
}

float4 GetLightProjectionUVW(in float3 worldpos, uint projectionIndex)
{
    float4 coord = mul(PK_BUFFER_DATA(pk_LightMatrices, projectionIndex), float4(worldpos, 1.0f));
    coord.xy = (coord.xy * 0.5f + coord.ww * 0.5f) / coord.w;
    return coord;
}


//----------LIGHT INDEXING----------//
void GetLight(uint index, in float3 worldpos, uint cascade, out float3 color, out float3 posToLight, out float attenuation)
{
    PKRawLight light = PK_BUFFER_DATA(pk_Lights, index);
    color = light.color.rgb;

    float2 lightuv;
    float linearDistance;

    // @TODO Maybe refactor lights to separate by type lists 
    switch (light.type)
    {
        case LIGHT_TYPE_POINT:
        {
            posToLight = light.position.xyz - worldpos;
            linearDistance = length(posToLight);
            attenuation = GetAttenuation(linearDistance, light.position.w);
            posToLight /= linearDistance;
            lightuv = OctaEncode(-posToLight);
        }
        break;
        case LIGHT_TYPE_SPOT:
        {
            posToLight = light.position.xyz - worldpos;
            linearDistance = length(posToLight);
            attenuation = GetAttenuation(linearDistance, light.position.w);
            posToLight /= linearDistance;

            float3 coord = GetLightProjectionUVW(worldpos, light.projection_index).xyz;
            lightuv = coord.xy;
            attenuation *= step(0.0f, coord.z);
            attenuation *= tex2D(pk_LightCookies, float3(lightuv, light.cookie_index)).r;
        }
        break;
        case LIGHT_TYPE_DIRECTIONAL:
        {
            light.projection_index += cascade;
            light.shadowmap_index += cascade;
            posToLight = -light.position.xyz;
            attenuation = 1.0f;

            float4 coord = GetLightProjectionUVW(worldpos, light.projection_index);
            linearDistance = ((coord.z / coord.w) + 1.0f) * light.position.w * 0.5f;
            lightuv = coord.xy;
        }
        break;
    }

    //if (light.cookie_index != LIGHT_PARAM_INVALID)
    //{
    //    attenuation *= tex2D(pk_LightCookies, float3(lightuv, light.cookie_index)).r;
    //}

    if (light.shadowmap_index != LIGHT_PARAM_INVALID)
    {
        attenuation *= SampleLightShadowmap(light.shadowmap_index, lightuv, linearDistance);
    }
}


PKLight GetSurfaceLightDirect(uint index, in float3 worldpos, uint cascade)
{
    float3 posToLight, color;
    float attenuation;
    GetLight(index, worldpos, cascade, color, posToLight, attenuation);
    return PKLight(color * attenuation, posToLight);
}

PKLight GetSurfaceLight(uint index, in float3 worldpos, uint cascade)
{
    float3 posToLight, color;
    float attenuation;
    GetLight(PK_BUFFER_DATA(pk_GlobalLightsList, index), worldpos, cascade, color, posToLight, attenuation);
    return PKLight(color * attenuation, posToLight);
}

float3 GetVolumeLightColor(uint index, in float3 worldpos, float3 viewdir, uint cascade, float anisotropy)
{
    float3 posToLight, color;
    float attenuation;
    GetLight(PK_BUFFER_DATA(pk_GlobalLightsList, index), worldpos, cascade, color, posToLight, attenuation);
    return color * attenuation * GetLightAnisotropy(viewdir, posToLight, anisotropy);
}

LightTile GetLightTile(int3 coord)
{
    #if defined(PK_WRITE_CLUSTER_LIGHTS)
        return LightTile(0,0,0);
    #else
        return CreateLightTile(imageLoad(pk_LightTiles, coord).x);
    #endif
}

LightTile GetLightTile(float3 clipuvw) { return GetLightTile(GetTileIndexUV(clipuvw.xy, LinearizeDepth(clipuvw.z))); }