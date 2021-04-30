#pragma once
#include HLSLSupport.glsl

uniform float pk_ClusterSizePx;

#define CLUSTER_TILE_COUNT_X 16
#define CLUSTER_TILE_COUNT_Y 9
#define CLUSTER_TILE_COUNT_Z 24
#define CLUSTER_TILE_COUNT_XY 144 // 16 * 9
#define CLUSTER_TILE_COUNT_MAX (16 * 9 * 24)
#define CLUSTER_SIZE_PX pk_ClusterSizePx

float ZCoordToLinearDepth(float index)
{
    return pk_ProjectionParams.x * pow(pk_ExpProjectionParams.z, index / CLUSTER_TILE_COUNT_Z);
}

uint GetDepthTileIndex(float2 pxcoord)
{
    return uint(uint(pxcoord.x / CLUSTER_SIZE_PX) + CLUSTER_TILE_COUNT_X * uint(pxcoord.y / CLUSTER_SIZE_PX));
}

int3 GetTileIndex(float2 pxcoord, float lineardepth)
{
    // Source: http://www.aortiz.me/2018/12/21/CG.html
    int zTile = int(log2(lineardepth) * (CLUSTER_TILE_COUNT_Z * pk_ExpProjectionParams.x) + (CLUSTER_TILE_COUNT_Z * pk_ExpProjectionParams.y));
    return int3(int2(pxcoord / CLUSTER_SIZE_PX), max(zTile, 0));
}

int3 GetTileIndexFragment()
{
    #if defined(SHADER_STAGE_FRAGMENT)
        return GetTileIndex(gl_FragCoord.xy, LinearizeDepth(gl_FragCoord.z));
    #else
        return int3(0);
    #endif
}