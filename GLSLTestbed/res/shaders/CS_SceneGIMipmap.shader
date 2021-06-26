#version 460
#pragma PROGRAM_COMPUTE
#include includes/HLSLSupport.glsl

uniform int pk_SampleLevel;
uniform sampler3D pk_MipSource;
layout(rgba16) uniform writeonly image3D pk_MipTarget;

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
void main()
{
    float3 uvw = (float3(gl_GlobalInvocationID) + 0.5f.xxx) / float3(gl_NumWorkGroups * gl_WorkGroupSize);
    imageStore(pk_MipTarget, int3(gl_GlobalInvocationID), tex2DLod(pk_MipSource, uvw, pk_SampleLevel));
}