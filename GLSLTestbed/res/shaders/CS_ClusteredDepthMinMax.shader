#version 460

#pragma PROGRAM_COMPUTE
#include includes/LightingCommon.glsl
#include includes/Reconstruction.glsl
#include includes/ClusteringCommon.glsl

layout(local_size_x = DEPTH_BATCH_SIZE_PX, local_size_y = DEPTH_BATCH_SIZE_PX, local_size_z = 1) in;
void main()
{
    uint2 pxcoord = gl_GlobalInvocationID.xy;
    pxcoord.x = min(pxcoord.x, uint(pk_ScreenParams.x - 1));
    pxcoord.y = min(pxcoord.y, uint(pk_ScreenParams.y - 1));

    float depth = LinearizeDepth(texelFetch(pk_ScreenDepth, int2(pxcoord), 0).r);

    uint zTile = min(23, uint(max(log2(depth) * pk_FrustumTileScaleBias.x + pk_FrustumTileScaleBias.y, 0.0)));
    uint3 tiles = uint3(uint2(pxcoord / pk_FrustumTileSizes[3]), zTile);
    uint tileIndex  = uint(tiles.x + TILE_GRID_X * tiles.y + (TILE_GRID_X * TILE_GRID_Y) * tiles.z);

    uint encodedDepth = floatBitsToUint(depth);

    atomicMin(PK_BUFFER_DATA(pk_FDepthRanges, tileIndex).depthmin, encodedDepth);
    atomicMax(PK_BUFFER_DATA(pk_FDepthRanges, tileIndex).depthmax, encodedDepth);
}