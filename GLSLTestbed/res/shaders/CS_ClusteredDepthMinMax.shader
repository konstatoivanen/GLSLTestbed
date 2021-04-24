#version 460
#pragma PROGRAM_COMPUTE
#include includes/Reconstruction.glsl
#include includes/ClusteringCommon.glsl

layout(local_size_x = DEPTH_BATCH_SIZE_PX, local_size_y = DEPTH_BATCH_SIZE_PX, local_size_z = 1) in;
void main()
{
    uint2 pxcoord = gl_GlobalInvocationID.xy;
    pxcoord.x = min(pxcoord.x, uint(pk_ScreenParams.x - 1));
    pxcoord.y = min(pxcoord.y, uint(pk_ScreenParams.y - 1));

    float depth = LinearizeDepth(texelFetch(pk_ScreenDepth, int2(pxcoord), 0).r);

    uint tileIndex = uint(uint(pxcoord.x / CLUSTER_SIZE_PX) + CLUSTER_TILE_COUNT_X * uint(pxcoord.y / CLUSTER_SIZE_PX));

    uint encodedDepth = floatBitsToUint(depth);

    atomicMin(PK_BUFFER_DATA(pk_FDepthRanges, tileIndex).depthmin, encodedDepth);
    atomicMax(PK_BUFFER_DATA(pk_FDepthRanges, tileIndex).depthmax, encodedDepth);
}