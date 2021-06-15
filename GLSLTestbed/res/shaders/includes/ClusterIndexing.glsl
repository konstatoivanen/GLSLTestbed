#pragma once
#include HLSLSupport.glsl

#if defined(PK_IMPORT_CLUSTER_DATA)
    PK_DECLARE_BUFFER(uint, pk_TileMaxDepths);
    PK_DECLARE_READONLY_BUFFER(float4, pk_LightDirections);
    PK_DECLARE_ATOMIC_VARIABLE(uint, pk_GlobalListListIndex);
    #define LOAD_MAX_DEPTH(index) uintBitsToFloat(PK_BUFFER_DATA(pk_TileMaxDepths, index))
#endif

#define CLUSTER_TILE_COUNT_X 16
#define CLUSTER_TILE_COUNT_Y 9
#define CLUSTER_TILE_COUNT_Z 24
#define CLUSTER_TILE_COUNT_XY float2(16.0f, 9.0f)
#define CLUSTER_GROUP_SIZE_Z 4
#define CLUSTER_GROUP_SIZE_XYZ 576 // 16 * 9 * 4
#define CLUSTER_DEPTH_BATCH_SIZE_PX 16
#define CLUSTER_TILE_MAX_LIGHT_COUNT 128

float ZCoordToLinearDepth(float index)
{
    return pk_ProjectionParams.x * pow(pk_ExpProjectionParams.z, index / CLUSTER_TILE_COUNT_Z);
}

uint GetDepthTileIndexUV(float2 uv)
{
    return uint(uint(uv.x * CLUSTER_TILE_COUNT_X) + CLUSTER_TILE_COUNT_X * uint(uv.y * CLUSTER_TILE_COUNT_Y));
}

int3 GetTileIndexUV(float2 uv, float lineardepth)
{
    // Source: http://www.aortiz.me/2018/12/21/CG.html
    int zTile = int(log2(lineardepth) * (CLUSTER_TILE_COUNT_Z * pk_ExpProjectionParams.x) + (CLUSTER_TILE_COUNT_Z * pk_ExpProjectionParams.y));
    return int3(int2(uv * CLUSTER_TILE_COUNT_XY), max(zTile, 0));
}