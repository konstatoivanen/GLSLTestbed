#version 460
#pragma PROGRAM_COMPUTE
#include includes/PKCommon.glsl

layout(rg32ui) uniform uimage2D pk_FlowTexture;
PK_DECLARE_BUFFER(float4, pk_Boids);

float3 GetTargetVelocity(int2 coord)
{
    float2 c = float2(imageLoad(pk_FlowTexture, coord).xy);
    float2 x0 = float2(imageLoad(pk_FlowTexture, coord + int2(1, 0)).xy);
    float2 x1 = float2(imageLoad(pk_FlowTexture, coord + int2(-1, 0)).xy);
    float2 y0 = float2(imageLoad(pk_FlowTexture, coord + int2(0, 1)).xy);
    float2 y1 = float2(imageLoad(pk_FlowTexture, coord + int2(0, -1)).xy);

    x0.x = lerp(x0.x, c.x, x0.y * 0.999f);
    x1.x = lerp(x1.x, c.x, x1.y * 0.999f);
    y0.x = lerp(y0.x, c.x, y0.y * 0.999f);
    y1.x = lerp(y1.x, c.x, y1.y * 0.999f);

    float border = max(max(max(x0.y, x1.y), y0.y), y1.y);
    float2 direction = clamp(float2(x0.x - x1.x, y0.x - y1.x), -1.0f.xx, 1.0f.xx);

    return float3(direction, border);
}

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
void main()
{
    float2 size = imageSize(pk_FlowTexture).xy;
    float4 boid = PK_BUFFER_DATA(pk_Boids, gl_GlobalInvocationID.x);
    float2 position = boid.xy;
    float2 velocity = boid.zw;

    int2 coord = int2(position * size);

    if (position.x < 0.01f)
    {
        position = float2(0.999f, GlobalNoiseBlue(uint2(coord + gl_GlobalInvocationID.xy)).x);
        velocity = 0.0f.xx;
    }

    coord = int2(position * size);
    
    int xoffset = int(imageLoad(pk_FlowTexture, coord + int2(1, 0)).x) - int(imageLoad(pk_FlowTexture, coord + int2(-1, 0)).x);
    int yoffset = int(imageLoad(pk_FlowTexture, coord + int2(0, 1)).x) - int(imageLoad(pk_FlowTexture, coord + int2(0, -1)).x);

    float3 target = GetTargetVelocity(coord);

    velocity = lerp(velocity, target.xy * 0.2f / size.xy, 0.015f + target.z * 0.5f);

    position.xy += velocity;
    position.y = clamp(position.y, 0.0f, 1.0f);

    PK_BUFFER_DATA(pk_Boids, gl_GlobalInvocationID.x) = float4(position, velocity);
}