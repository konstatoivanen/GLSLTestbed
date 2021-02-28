#pragma once
// This is a bit painful but whatever
#include <glad/glad.h>
#include <glm.hpp>
#include <ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define CG_TYPE_ERROR 0xFFFF

#define CG_TYPE_NONE 0
#define CG_TYPE_FLOAT 1
#define CG_TYPE_FLOAT2 2
#define CG_TYPE_FLOAT3 3
#define CG_TYPE_FLOAT4 4
#define CG_TYPE_FLOAT2X2 5
#define CG_TYPE_FLOAT3X3 6
#define CG_TYPE_FLOAT4X4 7
#define CG_TYPE_INT 8
#define CG_TYPE_INT2 9
#define CG_TYPE_INT3 10
#define CG_TYPE_INT4 11
#define CG_TYPE_TEXTURE 12
#define CG_TYPE_CONSTANT_BUFFER 13

#define CG_TYPE_SIZE_FLOAT 4
#define CG_TYPE_SIZE_FLOAT2 8		// 4 * 2
#define CG_TYPE_SIZE_FLOAT3 12		// 4 * 3
#define CG_TYPE_SIZE_FLOAT4 16		// 4 * 4
#define CG_TYPE_SIZE_FLOAT2X2 16	// 4 * 2 * 2
#define CG_TYPE_SIZE_FLOAT3X3 36	// 4 * 3 * 3
#define CG_TYPE_SIZE_FLOAT4X4 64	// 4 * 4 * 4
#define CG_TYPE_SIZE_INT 4
#define CG_TYPE_SIZE_INT2 8			// 4 * 2
#define CG_TYPE_SIZE_INT3 12		// 4 * 3
#define CG_TYPE_SIZE_INT4 16		// 4 * 4
#define CG_TYPE_SIZE_TEXTURE 4
#define CG_TYPE_SIZE_CONSTANT_BUFFER 4

#define CG_TYPE_COMPONENTS_FLOAT 1
#define CG_TYPE_COMPONENTS_FLOAT2 2
#define CG_TYPE_COMPONENTS_FLOAT3 3
#define CG_TYPE_COMPONENTS_FLOAT4 4
#define CG_TYPE_COMPONENTS_FLOAT2X2 2
#define CG_TYPE_COMPONENTS_FLOAT3X3 3
#define CG_TYPE_COMPONENTS_FLOAT4X4 4
#define CG_TYPE_COMPONENTS_INT 1
#define CG_TYPE_COMPONENTS_INT2 2
#define CG_TYPE_COMPONENTS_INT3 3
#define CG_TYPE_COMPONENTS_INT4 4
#define CG_TYPE_COMPONENTS_TEXTURE 1
#define CG_TYPE_COMPONENTS_CONSTANT_BUFFER 1

typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

typedef glm::vec2 float2;
typedef glm::vec3 float3;
typedef glm::vec4 float4;

typedef glm::mat2x2 float2x2;
typedef glm::mat3x3 float3x3;
typedef glm::mat4x4 float4x4;

typedef glm::ivec2 int2;
typedef glm::ivec3 int3;
typedef glm::ivec4 int4;

typedef glm::uvec2 uint2;
typedef glm::uvec3 uint3;
typedef glm::uvec4 uint4;

typedef glm::bvec2 bool2;
typedef glm::bvec3 bool3;
typedef glm::bvec4 bool4;

typedef glm::lowp_i8vec4 color32;
typedef glm::ivec4 color;

typedef glm::quat quaternion;

const float2 CG_FLOAT2_ZERO = { 0.0f, 0.0f };
const float3 CG_FLOAT3_ZERO = { 0.0f, 0.0f, 0.0f };
const float4 CG_FLOAT4_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };

const int2 CG_INT2_ZERO = { 0, 0 };
const int3 CG_INT3_ZERO = { 0, 0, 0 };
const int4 CG_INT4_ZERO = { 0, 0, 0, 0 };

const uint2 CG_UINT2_ZERO = { 0, 0 };
const uint3 CG_UINT3_ZERO = { 0, 0, 0 };
const uint4 CG_UINT4_ZERO = { 0, 0, 0, 0 };

const bool2 CG_BOOL2_ZERO = { false, false };
const bool3 CG_BOOL3_ZERO = { false, false, false };
const bool4 CG_BOOL4_ZERO = { false, false, false, false };

const float2 CG_FLOAT2_ONE = { 1.0f, 1.0f };
const float3 CG_FLOAT3_ONE = { 1.0f, 1.0f, 1.0f };
const float4 CG_FLOAT4_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };

const int2 CG_INT2_ONE = { 1, 1 };
const int3 CG_INT3_ONE = { 1, 1, 1 };
const int4 CG_INT4_ONE = { 1, 1, 1, 1 };

const uint2 CG_UINT2_ONE = { 1, 1 };
const uint3 CG_UINT3_ONE = { 1, 1, 1 };
const uint4 CG_UINT4_ONE = { 1, 1, 1, 1 };

const bool2 CG_BOOL2_ONE = { true, true };
const bool3 CG_BOOL3_ONE = { true, true, true };
const bool4 CG_BOOL4_ONE = { true, true, true, true };

const float3 CG_FLOAT3_LEFT     = {  1.0f,  0.0f,  0.0f };
const float3 CG_FLOAT3_RIGHT    = { -1.0f,  0.0f,  0.0f };
const float3 CG_FLOAT3_UP       = {  0.0f,  1.0f,  0.0f };
const float3 CG_FLOAT3_DOWN     = {  0.0f, -1.0f,  0.0f };
const float3 CG_FLOAT3_FORWARD  = {  0.0f,  0.0f,  1.0f };
const float3 CG_FLOAT3_BACKWARD = {  0.0f,  0.0f, -1.0f };

const quaternion CG_QUATERNION_IDENTITY =  { 1.0f, 0.0f, 0.0f, 0.0f };

const float4x4 CG_FLOAT4X4_IDENTITY = float4x4(1.0f);

const color CG_COLOR_WHITE   = { 1.0f, 1.0f, 1.0f, 1.0f };
const color CG_COLOR_GRAY    = { 0.5f, 0.5f, 0.5f, 1.0f };
const color CG_COLOR_BLACK   = { 0.0f, 0.0f, 0.0f, 1.0f };
const color CG_COLOR_CLEAR   = { 0.0f, 0.0f, 0.0f, 0.0f };
const color CG_COLOR_RED     = { 1.0f, 0.0f, 0.0f, 1.0f };
const color CG_COLOR_GREEN   = { 0.0f, 1.0f, 0.0f, 1.0f };
const color CG_COLOR_BLUE    = { 0.0f, 0.0f, 1.0f, 1.0f };
const color CG_COLOR_CYAN    = { 0.0f, 1.0f, 1.0f, 1.0f };
const color CG_COLOR_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
const color CG_COLOR_YELLOW  = { 1.0f, 1.0f, 0.0f, 1.0f };

const color32 CG_COLOR32_WHITE   = { 255, 255, 255, 255 };
const color32 CG_COLOR32_GRAY    = { 128, 128, 128, 255 };
const color32 CG_COLOR32_BLACK   = {   0,   0,   0, 255 };
const color32 CG_COLOR32_CLEAR   = {   0,   0,   0,   0 };
const color32 CG_COLOR32_RED     = { 255,   0,   0, 255 };
const color32 CG_COLOR32_GREEN   = {   0, 255,   0, 255 };
const color32 CG_COLOR32_BLUE    = {   0,   0, 255, 255 };
const color32 CG_COLOR32_CYAN    = {   0, 255, 255, 255 };
const color32 CG_COLOR32_MAGENTA = { 255,   0, 255, 255 };
const color32 CG_COLOR32_YELLOW  = { 255, 255,   0, 255 };

const float CG_FLOAT_PI = 3.14159274F;
const float CG_FLOAT_DEG2RAD = 0.0174532924F;
const float CG_FLOAT_RAD2DEG = 57.29578F;

class CGType
{
    public:
        static ushort Size(ushort type);
        static ushort Components(ushort type);
        static ushort BaseType(ushort type);
};

namespace CGMath
{
    inline float4x4 GetMatrixTRS(const float3& position, const quaternion& rotation, const float3& scale)
    {
        return glm::translate(float4x4(1.0f), position) * glm::toMat4(rotation) * glm::scale(scale);
    }

    inline float4x4 GetMatrixInvTRS(const float3& position, const quaternion& rotation, const float3& scale)
    {
        return glm::inverse(glm::translate(float4x4(1.0f), position) * glm::toMat4(rotation) * glm::scale(scale));
    }

    inline float4x4 GetMatrixTR(const float3& position, const quaternion& rotation)
    {
        return glm::translate(float4x4(1.0f), position) * glm::toMat4(rotation);
    }

    inline float4x4 GetPerspective(float fov, float aspect, float nearClip, float farClip)
    {
        auto proj = glm::perspective(fov * CG_FLOAT_DEG2RAD, aspect, nearClip, farClip);
        proj[2][2] *= -1;
        proj[2][3] *= -1;
        return proj;
    }

    inline float Cot(float value)
    {
        return cos(value) / sin(value);
    }
};