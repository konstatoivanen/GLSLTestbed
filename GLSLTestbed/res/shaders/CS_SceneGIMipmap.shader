#version 460
#pragma PROGRAM_COMPUTE
#include includes/HLSLSupport.glsl

uniform sampler3D pk_MipSource;
layout(rgba16) uniform writeonly restrict image3D pk_MipTarget;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
void main()
{
    int level = int(log2(textureSize(pk_MipSource, 0).x) - log2(gl_NumWorkGroups.x * gl_WorkGroupSize.x)) - 1;
    float3 uvw = (float3(gl_GlobalInvocationID) + 0.5f.xxx) / float3(gl_NumWorkGroups * gl_WorkGroupSize);
    imageStore(pk_MipTarget, int3(gl_GlobalInvocationID), tex2DLod(pk_MipSource, uvw, level));
}