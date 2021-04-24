#version 460

#pragma PROGRAM_COMPUTE
#include includes/ClusteringCommon.glsl

layout(local_size_x = CLUSTER_TILE_COUNT_X, local_size_y = CLUSTER_TILE_COUNT_Y, local_size_z = 1) in;
void main()
{
    PK_BUFFER_DATA(pk_FDepthRanges, gl_LocalInvocationIndex) = TileDepth(0xFFFFFFFF, 0);
}