#ifndef HLSL_SUPPORT
#define HLSL_SUPPORT

#define bool2 bvec2
#define bool3 bvec3
#define bool4 bvec4

#define half float
#define half2 vec2
#define half3 vec3
#define half4 vec4

#define float float
#define float2 vec2
#define float3 vec3
#define float4 vec4

#define double float 
#define double2 vec2
#define double3 vec3
#define double4 vec4

#define short int
#define short2 ivec2
#define short3 ivec3
#define short4 ivec4

#define int int
#define int2 int2
#define int3 int3
#define int4 int4

#define half2x2 mat2
#define half3x3 mat3
#define half4x4 mat4

#define float2x2 mat2
#define float3x3 mat3
#define float4x4 mat4

#define double2x2 mat2
#define double3x3 mat3
#define double4x4 mat4

#define saturate(v) clamp(v, 0.0, 1.0)

#define lerp(a,b,v) mix(a,b,v)

#define mul(a,b) (a * b)

#define tex2D(a,b) texture(a,b)

#define GEqual(a, b) any(greaterThanEqual(a,b))

#define LEqual(a, b) any(lessThanEqual(a,b))

#define Less(a,b) any(lessThan(a,b))

#define Greater(a,b) any(greaterThan(a,b))

#endif