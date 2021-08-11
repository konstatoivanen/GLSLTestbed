#version 460
#pragma PROGRAM_COMPUTE
#include includes/PKCommon.glsl

uniform uint pk_Command;
layout(rg32ui) uniform writeonly uimage2D pk_FlowTexture;

layout(local_size_x = 2, local_size_y = 2, local_size_z = 1) in;
void main()
{
    float2 uv = pk_CursorParams.xy / pk_ScreenParams.xy;
    uv.y = 1.0f - uv.y;

    uint4 value = pk_Command == 1 ? uint4(0, 1, 0, 0) : uint4(0,0,0,0);

    imageStore(pk_FlowTexture, int2(uv * imageSize(pk_FlowTexture).xy) + int2(gl_GlobalInvocationID.xy), value);
}