#version 460
#pragma PROGRAM_COMPUTE
#include includes/PKCommon.glsl

layout(r32ui) uniform uimage2D pk_FlowTexture;
uniform sampler2D pk_Maze;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
    int2 coord = int2(gl_GlobalInvocationID.xy);

    if (coord.x + coord.y == 0)
    {
        imageStore(pk_FlowTexture, int2((pk_CursorParams.xy / pk_ScreenParams.xy) * imageSize(pk_FlowTexture).xy), uint4(0xFFFFFF, 0, 0, 0));
    }

    if (texelFetch(pk_Maze, coord, 0).r > 0)
    {
        imageStore(pk_FlowTexture, coord, 0u.xxxx);
        return;
    }

    uint n0 = imageLoad(pk_FlowTexture, coord + int2(1,  0)).x;
    uint n1 = imageLoad(pk_FlowTexture, coord + int2(0,  1)).x;
    uint n2 = imageLoad(pk_FlowTexture, coord + int2(0, -1)).x;
    uint n3 = imageLoad(pk_FlowTexture, coord + int2(-1, 0)).x;

    uint maxvalue = max(max(max(n0,n1),n2),n3);

    if (maxvalue == 0)
    {
        return;
    }

    imageStore(pk_FlowTexture, coord, uint4(max(maxvalue - 1, 0), 0, 0, 0));
}