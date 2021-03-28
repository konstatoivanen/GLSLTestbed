#pragma once
#include HLSLSupport.glsl
#include ClusterIndexing.glsl

struct ClusterDispatchInfo
{
    uint groupsX;
    uint groupsY;
    uint groupsZ;
    uint clusterCount;
    uint lightIndexCount;
};

struct TileDepth
{
    uint depthmin;
    uint depthmax;
};

PK_DECLARE_BUFFER(TileDepth, pk_FDepthRanges);
PK_DECLARE_RESTRICTED_BUFFER(uint, pk_VisibleClusters);
PK_DECLARE_ATOMIC_VARIABLE(ClusterDispatchInfo, pk_ClusterDispatchInfo);

#define CLUSTER_TILE_MAX_LIGHT_COUNT 128
#define CLUSTER_TILE_BATCH_SIZE 32
#define DEPTH_BATCH_SIZE_PX 16