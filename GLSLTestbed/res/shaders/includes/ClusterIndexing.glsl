#pragma once
#include HLSLSupport.glsl

uniform float[5] pk_ClusterFrustumInfo;

#define CLUSTER_TILE_COUNT_X 16
#define CLUSTER_TILE_COUNT_Y 9
#define CLUSTER_TILE_COUNT_Z 24
#define CLUSTER_TILE_COUNT_XY 144 // 16 * 9
#define CLUSTER_TILE_COUNT_MAX (16 * 9 * 24)
#define CLUSTER_DEPTH_SCALE pk_ClusterFrustumInfo[0]
#define CLUSTER_DEPTH_BIAS pk_ClusterFrustumInfo[1]
#define CLUSTER_SIZE_PX pk_ClusterFrustumInfo[2]
#define CLUSTER_ZNEAR pk_ClusterFrustumInfo[3]
#define CLUSTER_ZFARNEAR pk_ClusterFrustumInfo[4]

uint LinearDepthToZCoord(float depth)
{
    return uint(max(log2(depth) * CLUSTER_DEPTH_SCALE + CLUSTER_DEPTH_BIAS, 0.0));
}

float ZCoordToLinearDepth(float index)
{
    return CLUSTER_ZNEAR * pow(CLUSTER_ZFARNEAR, index / CLUSTER_TILE_COUNT_Z);
}

uint2 ScreenToCoord2D(float2 screenpos)
{
    return uint2(screenpos / CLUSTER_SIZE_PX);
}

uint2 TileToCoord2D(uint index)
{
    return uint2(index % CLUSTER_TILE_COUNT_X, (index / CLUSTER_TILE_COUNT_X) % CLUSTER_TILE_COUNT_Y);
}

uint CoordToTile(uint3 coord)
{
    return uint(coord.x + CLUSTER_TILE_COUNT_X * coord.y + CLUSTER_TILE_COUNT_XY * coord.z); 
}

uint GetTileIndex(float2 pxcoord, float lineardepth)
{
    uint zTile = uint(max(log2(lineardepth) * CLUSTER_DEPTH_SCALE + CLUSTER_DEPTH_BIAS, 0.0));
    uint3 tiles = uint3(uint2(pxcoord / CLUSTER_SIZE_PX), zTile);
    return uint(tiles.x + CLUSTER_TILE_COUNT_X * tiles.y + CLUSTER_TILE_COUNT_XY * tiles.z); 
}

uint GetTileIndexFragment()
{
    // Source: http://www.aortiz.me/2018/12/21/CG.html
    #if defined(SHADER_STAGE_FRAGMENT)
        uint zTile = uint(max(log2(LinearizeDepth(gl_FragCoord.z)) * CLUSTER_DEPTH_SCALE + CLUSTER_DEPTH_BIAS, 0.0));
        uint3 tiles = uint3( uint2( gl_FragCoord.xy / CLUSTER_SIZE_PX), zTile);
        return  uint(tiles.x + CLUSTER_TILE_COUNT_X * tiles.y + CLUSTER_TILE_COUNT_XY * tiles.z); 
    #else
        return 0;
    #endif
}