#pragma once
#include HLSLSupport.glsl
#include ClusterIndexing.glsl

PK_DECLARE_BUFFER(uint, pk_TileMaxDepths);
PK_DECLARE_READONLY_BUFFER(float4, pk_LightDirections);
PK_DECLARE_ATOMIC_VARIABLE(uint, pk_GlobalListListIndex);

#define LOAD_MAX_DEPTH(index) uintBitsToFloat(PK_BUFFER_DATA(pk_TileMaxDepths, index))

#define CLUSTER_TILE_MAX_LIGHT_COUNT 128
#define DEPTH_BATCH_SIZE_PX_X 16
#define DEPTH_BATCH_SIZE_PX_Y 16