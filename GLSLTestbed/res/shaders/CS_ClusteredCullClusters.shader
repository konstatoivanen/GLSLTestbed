#version 460

#multi_compile PASS_CLUSTERS PASS_DISPATCH

#pragma PROGRAM_COMPUTE
#include includes/PKCommon.glsl
#include includes/LightingCommon.glsl
#include includes/ClusteringCommon.glsl

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    #if defined(PASS_CLUSTERS)
        uint tileIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

        PK_BUFFER_DATA(pk_LightTiles, tileIndex) = 0;

        TileDepth tileDepth = PK_BUFFER_DATA(pk_FDepthRanges, tileIndex);

        float minDepth = uintBitsToFloat(tileDepth.depthmin);
        float maxDepth = uintBitsToFloat(tileDepth.depthmax);
        
        float pnear = pk_ProjectionParams.y;
        float pfar = pk_ProjectionParams.z;
        
        float near = pnear * pow(pfar / pnear, gl_WorkGroupID.z / float(gl_NumWorkGroups.z));
        float far = pnear * pow(pfar / pnear, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));
        
        float2 screenmin = float2(gl_WorkGroupID.xy) * pk_FrustumTileSizes[3];
        
        uint count = minDepth < far && maxDepth > near && Less(screenmin, pk_ScreenParams.xy) ? 1 : 0;

        uint offset = atomicAdd(PK_ATOMIC_DATA(pk_ClusterDispatchInfo).clusterCount, count);
        
        if (count > 0)
        {
            PK_BUFFER_DATA(pk_VisibleClusters, offset) = tileIndex;
        }

    #else
        ClusterDispatchInfo info = PK_ATOMIC_DATA(pk_ClusterDispatchInfo);
        info.lightIndexCount = 0;
        info.groupsX = uint(ceil(info.clusterCount / float(TILE_BATCH_SIZE)));
        info.groupsY = 1;
        info.groupsZ = 1;

        PK_BUFFER_DATA(pk_VisibleClusters, info.clusterCount) = TILE_MAX_COUNT;
        PK_ATOMIC_DATA(pk_ClusterDispatchInfo) = info;
    #endif
}