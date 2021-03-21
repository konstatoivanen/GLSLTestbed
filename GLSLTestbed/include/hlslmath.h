#pragma once
#define GLM_FORCE_SWIZZLE 
#include <glad/glad.h>
#include <glm.hpp>
#include <ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

// @TODO Replace with enums
namespace PK::Math
{
    enum class CG_TYPE
    {
        NONE = 0,
        FLOAT = 1,
        FLOAT2 = 2,
        FLOAT3 = 3,
        FLOAT4 = 4,
        FLOAT2X2 = 5,
        FLOAT3X3 = 6,
        FLOAT4X4 = 7,
        INT = 8,
        INT2 = 9,
        INT3 = 10,
        INT4 = 11,
        TEXTURE = 12,
        CONSTANT_BUFFER = 13,
        COMPUTE_BUFFER = 14,
        VERTEX_ARRAY = 15,
        INVALID = 0xFFFF
    };
    
    const unsigned short CG_TYPE_SIZE_FLOAT = 4;
    const unsigned short CG_TYPE_SIZE_FLOAT2 = 8;		// 4 * 2
    const unsigned short CG_TYPE_SIZE_FLOAT3 = 12;		// 4 * 3
    const unsigned short CG_TYPE_SIZE_FLOAT4 = 16;		// 4 * 4
    const unsigned short CG_TYPE_SIZE_FLOAT2X2 = 16; // 4 * 2 * 2
    const unsigned short CG_TYPE_SIZE_FLOAT3X3 = 36;	// 4 * 3 * 3
    const unsigned short CG_TYPE_SIZE_FLOAT4X4 = 64;	// 4 * 4 * 4
    const unsigned short CG_TYPE_SIZE_INT = 4;
    const unsigned short CG_TYPE_SIZE_INT2 = 8;			// 4 * 2
    const unsigned short CG_TYPE_SIZE_INT3 = 12;		// 4 * 3
    const unsigned short CG_TYPE_SIZE_INT4 = 16;		// 4 * 4
    const unsigned short CG_TYPE_SIZE_TEXTURE = 4;
    const unsigned short CG_TYPE_SIZE_CONSTANT_BUFFER = 4;
    const unsigned short CG_TYPE_SIZE_COMPUTE_BUFFER = 4;
    
    const unsigned short CG_TYPE_COMPONENTS_FLOAT = 1;
    const unsigned short CG_TYPE_COMPONENTS_FLOAT2 = 2;
    const unsigned short CG_TYPE_COMPONENTS_FLOAT3 = 3;
    const unsigned short CG_TYPE_COMPONENTS_FLOAT4 = 4;
    const unsigned short CG_TYPE_COMPONENTS_FLOAT2X2 = 2;
    const unsigned short CG_TYPE_COMPONENTS_FLOAT3X3 = 3;
    const unsigned short CG_TYPE_COMPONENTS_FLOAT4X4 = 4;
    const unsigned short CG_TYPE_COMPONENTS_INT = 1;
    const unsigned short CG_TYPE_COMPONENTS_INT2 = 2;
    const unsigned short CG_TYPE_COMPONENTS_INT3 = 3;
    const unsigned short CG_TYPE_COMPONENTS_INT4 = 4;
    const unsigned short CG_TYPE_COMPONENTS_TEXTURE = 1;
    const unsigned short CG_TYPE_COMPONENTS_CONSTANT_BUFFER = 1;
    const unsigned short CG_TYPE_COMPONENTS_COMPUTE_BUFFER = 1;
    
    typedef uint16_t ushort;
    typedef uint32_t uint;
    typedef uint64_t ulong;
    typedef signed char sbyte;
    
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
    typedef glm::vec4 color;
    
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
    
    const float2 CG_FLOAT2_UP       = { 0.0f, 1.0f };
    const float2 CG_FLOAT2_DOWN     = { 0.0f, -1.0f };
    const float2 CG_FLOAT2_LEFT     = { -1.0f, 0.0f };
    const float2 CG_FLOAT2_RIGHT    = { 1.0f, 0.0f };
    
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
    const float CG_FLOAT_2PI = 2.0f * 3.14159274F;
    const float CG_FLOAT_DEG2RAD = 0.0174532924F;
    const float CG_FLOAT_RAD2DEG = 57.29578F;
    
    struct FrustrumPlanes
    {
        // left, right, top, bottom, near, far
        float4 planes[6];
    };
    
    struct BoundingBox
    {
        float3 min;
        float3 max;
    
        float3 GetCenter() const { return min + (max - min) * 0.5f; }
        float3 GetExtents() const { return (max - min) * 0.5f; }
    
        BoundingBox() : min(CG_FLOAT3_ZERO), max(CG_FLOAT3_ZERO) {}
        BoundingBox(const float3& _min, const float3& _max) : min(_min), max(_max) {}
    };
    
    namespace Convert
    {
        ushort Size(CG_TYPE type);
        ushort Components(CG_TYPE type);
        ushort BaseType(CG_TYPE type);
        ushort NativeEnum(CG_TYPE type);
        std::string ToString(CG_TYPE type);
        CG_TYPE FromString(const char* string);
    };
    
    namespace Functions
    {
        float4x4 GetMatrixTRS(const float3& position, const quaternion& rotation, const float3& scale);
        float4x4 GetMatrixInvTRS(const float3& position, const quaternion& rotation, const float3& scale);
        float4x4 GetMatrixTR(const float3& position, const quaternion& rotation);
        float4x4 GetPerspective(float fov, float aspect, float nearClip, float farClip);
        inline float GetSizePerDepth(float fovy) { return (float)tan(fovy * 0.5f * CG_FLOAT_DEG2RAD); }
        inline float GetSizeOnScreen(float depth, float sizePerDepth, float radius) { return radius / (sizePerDepth * depth); }
        float4x4 GetOffsetPerspective(float left, float right, float bottom, float top, float fovy, float aspect, float zNear, float zFar);
        float4x4 GetPerspectiveSubdivision(int index, const int3& gridSize, float fovy, float aspect, float znear, float zfar);
    
        color HueToRGB(float hue);
        inline float Cot(float value) { return cos(value) / sin(value); }
        inline float RandomFloat() { return (float)rand() / (float)RAND_MAX; }
        inline float3 RandomFloat3() { return float3(RandomFloat(), RandomFloat(), RandomFloat()); }
        inline float RandomRangeFloat(float min, float max) { return min + (RandomFloat() * (max - min)); }
        inline float3 RandomRangeFloat3(const float3& min, const float3& max) { return float3(RandomRangeFloat(min.x, max.x), RandomRangeFloat(min.y, max.y), RandomRangeFloat(min.z, max.z)); }
        inline float3 RandomEuler() { return float3(RandomRangeFloat(-360.0f, 360.0f), RandomRangeFloat(-360.0f, 360.0f), RandomRangeFloat(-360.0f, 360.0f)); }
        size_t GetNextExponentialSize(size_t start, size_t min);
        
        void NormalizePlane(float4* plane);
        void ExtractFrustrumPlanes(const float4x4 viewprojection, FrustrumPlanes* frustrum, bool normalize);
    
        inline float PlaneDistanceToPoint(const float4& plane, const float3& point) { return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w; }
    
        inline float3 IntesectPlanes3(const float4& p1, const float4& p2, const float4& p3)
        {
            float3 n1 = p1.xyz, n2 = p2.xyz, n3 = p3.xyz;
            return ((-p1.w * glm::cross(n2, n3)) + (-p2.w * glm::cross(n3, n1)) + (-p3.w * glm::cross(n1, n2))) / (glm::dot(n1, glm::cross(n2, n3)));
        }
    
        inline BoundingBox CreateBoundsMinMax(const float3& min, const float3& max) { return BoundingBox(min, max); }
        inline BoundingBox CreateBoundsCenterExtents(const float3& center, const float3& extents) { return BoundingBox(center - extents, center + extents); }
    
        bool IntersectPlanesAABB(const float4* planes, int planeCount, const BoundingBox& aabb);
        void BoundsEncapsulate(BoundingBox* bounds, const BoundingBox& other);
        int BoundsLongestAxis(const BoundingBox& bounds);
        int BoundsShortestAxis(const BoundingBox& bounds);
        void BoundsSplit(const BoundingBox& bounds, int axis, BoundingBox* out0, BoundingBox* out1);
        bool BoundsContains(const BoundingBox& bounds, const float3& point);
        BoundingBox BoundsTransform(const float4x4& matrix, const BoundingBox& bounds);
    };
}