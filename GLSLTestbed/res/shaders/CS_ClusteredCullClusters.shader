#version 460

#multi_compile PASS_CLUSTERS PASS_DISPATCH

#pragma PROGRAM_COMPUTE
#define PK_WRITE_CLUSTER_LIGHTS
#include includes/PKCommon.glsl
#include includes/LightingCommon.glsl
#include includes/ClusteringCommon.glsl

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    #if defined(PASS_CLUSTERS)
        uint tileIndex = CoordToTile(gl_WorkGroupID.xyz);

        TileDepth tileDepth = PK_BUFFER_DATA(pk_FDepthRanges, tileIndex);

        float minDepth = uintBitsToFloat(tileDepth.depthmin);
        float maxDepth = uintBitsToFloat(tileDepth.depthmax);
        
        float near = ZCoordToLinearDepth(gl_WorkGroupID.z);
        float far = ZCoordToLinearDepth(gl_WorkGroupID.z + 1);
        
        float2 screenmin = gl_WorkGroupID.xy * CLUSTER_SIZE_PX;
        
        uint count = minDepth < far && maxDepth > near && Less(screenmin, pk_ScreenParams.xy) ? 1 : 0;

        uint offset = atomicAdd(PK_ATOMIC_DATA(pk_ClusterDispatchInfo).clusterCount, count);
        
        if (count > 0)
        {
            PK_BUFFER_DATA(pk_VisibleClusters, offset) = tileIndex;
        }

    #else
        ClusterDispatchInfo info = PK_ATOMIC_DATA(pk_ClusterDispatchInfo);
        info.lightIndexCount = 0;
        info.groupsX = uint(ceil(info.clusterCount / float(CLUSTER_TILE_BATCH_SIZE)));
        info.groupsY = 1;
        info.groupsZ = 1;

        PK_BUFFER_DATA(pk_VisibleClusters, info.clusterCount) = CLUSTER_TILE_COUNT_MAX;
        PK_ATOMIC_DATA(pk_ClusterDispatchInfo) = info;
    #endif
}