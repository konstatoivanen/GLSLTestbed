#version 460
#pragma PROGRAM_COMPUTE
#include includes/HLSLSupport.glsl
#define KERNEL_SIZE 2 * 2 * 2

uniform int pk_SampleLevel;
uniform sampler3D pk_MipSource;
layout(rgba16) uniform writeonly image3D pk_MipTarget;

shared float4 sharedValues[KERNEL_SIZE];

layout(local_size_x = 2, local_size_y = 2, local_size_z = 2) in;
void main()
{
    sharedValues[gl_LocalInvocationIndex] = texelFetch(pk_MipSource, int3(gl_GlobalInvocationID), pk_SampleLevel) / KERNEL_SIZE;

    barrier();

    if (gl_LocalInvocationIndex != 0)
    {
        return;
    }

    float4 value = 0.0f.xxxx;

    for (int i = 0; i < KERNEL_SIZE; ++i)
    {
        value += sharedValues[i];
    }

    imageStore(pk_MipTarget, int3(gl_WorkGroupID), value);
}