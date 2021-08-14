#version 460
#pragma PROGRAM_COMPUTE
#include includes/PKCommon.glsl

layout(r32ui) uniform uimage2D pk_FlowTexture;
PK_DECLARE_BUFFER(float4, pk_Boids);

float2 GetTargetVelocity(int2 coord)
{
    float x0 = imageLoad(pk_FlowTexture, coord + int2(1, 0)).x;
    float x1 = imageLoad(pk_FlowTexture, coord + int2(-1, 0)).x;
    float y0 = imageLoad(pk_FlowTexture, coord + int2(0, 1)).x;
    float y1 = imageLoad(pk_FlowTexture, coord + int2(0, -1)).x;
    float2 direction = clamp(float2(x0.x - x1.x, y0.x - y1.x), -1.0f.xx, 1.0f.xx);
    return direction;
}

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
void main()
{
    float2 size = imageSize(pk_FlowTexture).xy;
    float4 boid = PK_BUFFER_DATA(pk_Boids, gl_GlobalInvocationID.x);
    float time = pk_Time.y;
    float2 position = boid.xy;
    float speed = boid.z;
    float killtime = boid.w;

    int2 coord = int2(position * size);

    if (time > killtime)
    {
        float3 noise = GlobalNoiseBlue(uint2(coord + gl_GlobalInvocationID.xy)).xyz;
        position = float2(noise.xy);

        killtime = time + 60.0f + (1.0f - noise.z) * 120.0f;
        speed = lerp(0.25f, 1.0f, noise.z);
    }

    coord = int2(position * size);
    
    int xoffset = int(imageLoad(pk_FlowTexture, coord + int2(1, 0)).x) - int(imageLoad(pk_FlowTexture, coord + int2(-1, 0)).x);
    int yoffset = int(imageLoad(pk_FlowTexture, coord + int2(0, 1)).x) - int(imageLoad(pk_FlowTexture, coord + int2(0, -1)).x);

    position.xy += GetTargetVelocity(coord) * speed.xx / size.xx;
    position.xy = clamp(position.xy, 0.0f, 1.0f);

    PK_BUFFER_DATA(pk_Boids, gl_GlobalInvocationID.x) = float4(position, speed, killtime);
}