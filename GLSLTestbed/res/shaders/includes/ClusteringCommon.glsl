#pragma once
#include HLSLSupport.glsl
#include ClusterIndexing.glsl

struct TileDepth
{
    uint depthmin;
    uint depthmax;
};

PK_DECLARE_BUFFER(TileDepth, pk_FDepthRanges);
PK_DECLARE_READONLY_BUFFER(float4, pk_LightDirections);
PK_DECLARE_ATOMIC_VARIABLE(uint, pk_GlobalListListIndex);
// Control z min max optimization as transparent objects & fx might need cells that don't touch any opaque surfaces. 
//#define CLUSTERING_CULL_OPTIMIZE_DEPTH

#define CLUSTER_TILE_MAX_LIGHT_COUNT 128
#define DEPTH_BATCH_SIZE_PX 16