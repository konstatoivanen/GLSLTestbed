#version 460

#pragma PROGRAM_COMPUTE

#include includes/PKCommon.glsl
#include includes/LightingCommon.glsl
#include includes/ClusteringCommon.glsl

shared float4 sharedLights[CLUSTER_TILE_BATCH_SIZE];

float cmax(float3 v) 
{
    return max(max(v.x, v.y), v.z);
}

layout(local_size_x = CLUSTER_TILE_BATCH_SIZE, local_size_y = 1, local_size_z = 1) in;
void main() 
{
    uint clusterCount = PK_ATOMIC_DATA(pk_ClusterDispatchInfo).clusterCount;
    uint numBatches = (pk_LightCount + CLUSTER_TILE_BATCH_SIZE - 1) / CLUSTER_TILE_BATCH_SIZE;
    uint tileIndex = PK_BUFFER_DATA(pk_VisibleClusters, min(gl_GlobalInvocationID.x, clusterCount));

    uint2 tileCoord = TileToCoord2D(tileIndex);

    TileDepth tileDepth = PK_BUFFER_DATA(pk_FDepthRanges, min(tileIndex, CLUSTER_TILE_COUNT_MAX - 1));
    float near = uintBitsToFloat(tileDepth.depthmin);
    float far = uintBitsToFloat(tileDepth.depthmax);

    float2 invstep = 1.0f / float2(CLUSTER_TILE_COUNT_X, CLUSTER_TILE_COUNT_X * (pk_ScreenParams.y / pk_ScreenParams.x));
    float4 screenminmax = float4(tileCoord.xy * invstep, (tileCoord.xy + 1.0f) * invstep);

    float3 min00 = ClipToViewPos(screenminmax.xy, near);
    float3 max00 = ClipToViewPos(screenminmax.xy, far);
    float3 min11 = ClipToViewPos(screenminmax.zw, near);
    float3 max11 = ClipToViewPos(screenminmax.zw, far);

    float3 aabbmin = min(min(min00, max00), min(min11, max11));
    float3 aabbmax = max(max(min00, max00), max(min11, max11));

    float3 cellextents = (aabbmax - aabbmin) * 0.5f;
    float3 cellcenter = aabbmin + cellextents;

    uint visibleLightCount = 0;
    uint visibleLightIndices[CLUSTER_TILE_MAX_LIGHT_COUNT];

    for (uint batch = 0; batch < numBatches; ++batch) 
    {
        uint lightIndex = min(batch * CLUSTER_TILE_BATCH_SIZE + gl_LocalInvocationIndex, pk_LightCount);

        sharedLights[gl_LocalInvocationIndex] = PK_BUFFER_DATA(pk_Lights, lightIndex).position;
        sharedLights[gl_LocalInvocationIndex].xyz = mul(pk_MATRIX_V, float4(sharedLights[gl_LocalInvocationIndex].xyz, 1.0f)).xyz;
        barrier();

        for (uint index = 0; index < CLUSTER_TILE_BATCH_SIZE && tileIndex != CLUSTER_TILE_COUNT_MAX && visibleLightCount < CLUSTER_TILE_MAX_LIGHT_COUNT; ++index)
        {
            float4 light = sharedLights[index];
            float3 d = abs(light.xyz - cellcenter) - cellextents;
            float r = light.w - cmax(min(d, float3(0.0f)));
            d = max(d, float3(0.0f));
    
            if (light.w > 0.0f && dot(d,d) <= r * r)
            {
                visibleLightIndices[visibleLightCount++] = batch * CLUSTER_TILE_BATCH_SIZE + index;
            }
        }
    }

    barrier();

    uint offset = atomicAdd(PK_ATOMIC_DATA(pk_ClusterDispatchInfo).lightIndexCount, visibleLightCount);

    for (uint i = 0; i < visibleLightCount; ++i) 
    {
        PK_BUFFER_DATA(pk_GlobalLightsList, offset + i) = visibleLightIndices[i];
    }

    if (tileIndex != CLUSTER_TILE_COUNT_MAX)
    {
        PK_BUFFER_DATA(pk_LightTiles, tileIndex) = (visibleLightCount << 24) | (offset & 0xFFFFFF);
    }
}
