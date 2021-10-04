#version 460

#Blend SrcAlpha OneMinusSrcAlpha

#define PK_IMPORT_CLUSTER_DATA
#include includes/PKCommon.glsl
#include includes/Reconstruction.glsl
#include includes/ClusterIndexing.glsl
#include includes/LightingCommon.glsl

#pragma PROGRAM_VERTEX
#define USE_VERTEX_PROGRAM_IMG
#include includes/BlitCommon.glsl

#pragma PROGRAM_FRAGMENT

in float2 vs_TEXCOORD0;
layout(location = 0) out float4 SV_Target0;

float3 hsv2rgb(float3 c)
{
	float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	float depth = LinearizeDepth(tex2D(pk_ScreenDepth, vs_TEXCOORD0).r);

	int3 tileCoord = GetTileIndexUV(vs_TEXCOORD0, depth);

	LightTile tile = CreateLightTile(imageLoad(pk_LightTiles, tileCoord).x);

	float maxfar = mod(LOAD_MAX_DEPTH(tileCoord.x + tileCoord.y * CLUSTER_TILE_COUNT_X) / 50, 1.0f);

	float maxcount = min(CLUSTER_TILE_MAX_LIGHT_COUNT, pk_LightCount);
	float tileIntensity = (tile.end - tile.start) / (maxcount * 0.75f);

	float4 color = float4(hsv2rgb(float3(tileIntensity, 1.0f, 1.0f)), 0.1f);

	float2 border = 4.0f / ceil(pk_ScreenParams.xy / CLUSTER_TILE_COUNT_XY);
	float2 griduv = vs_TEXCOORD0 * CLUSTER_TILE_COUNT_XY;
	griduv += border * 0.5f;
	griduv -= floor(griduv);
	color.xyz *= step(border.x, griduv.x);
	color.xyz *= step(border.y, griduv.y);

	SV_Target0 = color;
};