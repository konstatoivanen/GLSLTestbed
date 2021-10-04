#pragma once
#define GLM_FORCE_SWIZZLE 
#include <glad/glad.h>
#include <glm.hpp>
#include <ext.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace PK::Math
{
    enum class CG_TYPE : uint16_t
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
        UINT = 12,
        UINT2 = 13,
        UINT3 = 14,
        UINT4 = 15,
        HANDLE = 16,
        TEXTURE = 17,
        IMAGE_PARAMS = 18,
        CONSTANT_BUFFER = 19,
        COMPUTE_BUFFER = 20,
        VERTEX_ARRAY = 21,
        INVALID = 0xFFFF
    };
    
    constexpr unsigned short CG_TYPE_SIZE_FLOAT = 4;
    constexpr unsigned short CG_TYPE_SIZE_FLOAT2 = 8;		// 4 * 2
    constexpr unsigned short CG_TYPE_SIZE_FLOAT3 = 12;		// 4 * 3
    constexpr unsigned short CG_TYPE_SIZE_FLOAT4 = 16;		// 4 * 4
    constexpr unsigned short CG_TYPE_SIZE_FLOAT2X2 = 16; // 4 * 2 * 2
    constexpr unsigned short CG_TYPE_SIZE_FLOAT3X3 = 36;	// 4 * 3 * 3
    constexpr unsigned short CG_TYPE_SIZE_FLOAT4X4 = 64;	// 4 * 4 * 4
    constexpr unsigned short CG_TYPE_SIZE_INT = 4;
    constexpr unsigned short CG_TYPE_SIZE_INT2 = 8;			// 4 * 2
    constexpr unsigned short CG_TYPE_SIZE_INT3 = 12;		// 4 * 3
    constexpr unsigned short CG_TYPE_SIZE_INT4 = 16;		// 4 * 4
    constexpr unsigned short CG_TYPE_SIZE_HANDLE = 8;
    constexpr unsigned short CG_TYPE_SIZE_TEXTURE = 4;
    constexpr unsigned short CG_TYPE_SIZE_IMAGEPARAMS = 21;
    constexpr unsigned short CG_TYPE_SIZE_CONSTANT_BUFFER = 4;
    constexpr unsigned short CG_TYPE_SIZE_COMPUTE_BUFFER = 4;
    
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT = 1;
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT2 = 2;
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT3 = 3;
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT4 = 4;
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT2X2 = 2;
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT3X3 = 3;
    constexpr unsigned short CG_TYPE_COMPONENTS_FLOAT4X4 = 4;
    constexpr unsigned short CG_TYPE_COMPONENTS_INT = 1;
    constexpr unsigned short CG_TYPE_COMPONENTS_INT2 = 2;
    constexpr unsigned short CG_TYPE_COMPONENTS_INT3 = 3;
    constexpr unsigned short CG_TYPE_COMPONENTS_INT4 = 4;
    constexpr unsigned short CG_TYPE_COMPONENTS_HANDLE = 1;
    constexpr unsigned short CG_TYPE_COMPONENTS_TEXTURE = 1;
    constexpr unsigned short CG_TYPE_COMPONENTS_IMAGEPARAMS = 1;
    constexpr unsigned short CG_TYPE_COMPONENTS_CONSTANT_BUFFER = 1;
    constexpr unsigned short CG_TYPE_COMPONENTS_COMPUTE_BUFFER = 1;
    
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
    
    constexpr float2 CG_FLOAT2_ZERO = { 0.0f, 0.0f };
    constexpr float3 CG_FLOAT3_ZERO = { 0.0f, 0.0f, 0.0f };
    constexpr float4 CG_FLOAT4_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    constexpr int2 CG_INT2_ZERO = { 0, 0 };
    constexpr int3 CG_INT3_ZERO = { 0, 0, 0 };
    constexpr int4 CG_INT4_ZERO = { 0, 0, 0, 0 };
    
    constexpr uint2 CG_UINT2_ZERO = { 0, 0 };
    constexpr uint3 CG_UINT3_ZERO = { 0, 0, 0 };
    constexpr uint4 CG_UINT4_ZERO = { 0, 0, 0, 0 };
    
    constexpr bool2 CG_BOOL2_ZERO = { false, false };
    constexpr bool3 CG_BOOL3_ZERO = { false, false, false };
    constexpr bool4 CG_BOOL4_ZERO = { false, false, false, false };
    
    constexpr float2 CG_FLOAT2_ONE = { 1.0f, 1.0f };
    constexpr float3 CG_FLOAT3_ONE = { 1.0f, 1.0f, 1.0f };
    constexpr float4 CG_FLOAT4_ONE = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    constexpr int2 CG_INT2_ONE = { 1, 1 };
    constexpr int3 CG_INT3_ONE = { 1, 1, 1 };
    constexpr int4 CG_INT4_ONE = { 1, 1, 1, 1 };
    
    constexpr uint2 CG_UINT2_ONE = { 1, 1 };
    constexpr uint3 CG_UINT3_ONE = { 1, 1, 1 };
    constexpr uint4 CG_UINT4_ONE = { 1, 1, 1, 1 };
    
    constexpr bool2 CG_BOOL2_ONE = { true, true };
    constexpr bool3 CG_BOOL3_ONE = { true, true, true };
    constexpr bool4 CG_BOOL4_ONE = { true, true, true, true };
    
    constexpr float2 CG_FLOAT2_UP       = { 0.0f, 1.0f };
    constexpr float2 CG_FLOAT2_DOWN     = { 0.0f, -1.0f };
    constexpr float2 CG_FLOAT2_LEFT     = { -1.0f, 0.0f };
    constexpr float2 CG_FLOAT2_RIGHT    = { 1.0f, 0.0f };
    
    constexpr float3 CG_FLOAT3_LEFT     = {  1.0f,  0.0f,  0.0f };
    constexpr float3 CG_FLOAT3_RIGHT    = { -1.0f,  0.0f,  0.0f };
    constexpr float3 CG_FLOAT3_UP       = {  0.0f,  1.0f,  0.0f };
    constexpr float3 CG_FLOAT3_DOWN     = {  0.0f, -1.0f,  0.0f };
    constexpr float3 CG_FLOAT3_FORWARD  = {  0.0f,  0.0f,  1.0f };
    constexpr float3 CG_FLOAT3_BACKWARD = {  0.0f,  0.0f, -1.0f };
    
    constexpr quaternion CG_QUATERNION_IDENTITY =  { 1.0f, 0.0f, 0.0f, 0.0f };
    
    constexpr float4x4 CG_FLOAT4X4_IDENTITY = float4x4(1.0f);
    
    constexpr color CG_COLOR_WHITE   = { 1.0f, 1.0f, 1.0f, 1.0f };
    constexpr color CG_COLOR_GRAY    = { 0.5f, 0.5f, 0.5f, 1.0f };
    constexpr color CG_COLOR_BLACK   = { 0.0f, 0.0f, 0.0f, 1.0f };
    constexpr color CG_COLOR_CLEAR   = { 0.0f, 0.0f, 0.0f, 0.0f };
    constexpr color CG_COLOR_RED     = { 1.0f, 0.0f, 0.0f, 1.0f };
    constexpr color CG_COLOR_GREEN   = { 0.0f, 1.0f, 0.0f, 1.0f };
    constexpr color CG_COLOR_BLUE    = { 0.0f, 0.0f, 1.0f, 1.0f };
    constexpr color CG_COLOR_CYAN    = { 0.0f, 1.0f, 1.0f, 1.0f };
    constexpr color CG_COLOR_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
    constexpr color CG_COLOR_YELLOW  = { 1.0f, 1.0f, 0.0f, 1.0f };
    
    constexpr color32 CG_COLOR32_WHITE   = { 255, 255, 255, 255 };
    constexpr color32 CG_COLOR32_GRAY    = { 128, 128, 128, 255 };
    constexpr color32 CG_COLOR32_BLACK   = {   0,   0,   0, 255 };
    constexpr color32 CG_COLOR32_CLEAR   = {   0,   0,   0,   0 };
    constexpr color32 CG_COLOR32_RED     = { 255,   0,   0, 255 };
    constexpr color32 CG_COLOR32_GREEN   = {   0, 255,   0, 255 };
    constexpr color32 CG_COLOR32_BLUE    = {   0,   0, 255, 255 };
    constexpr color32 CG_COLOR32_CYAN    = {   0, 255, 255, 255 };
    constexpr color32 CG_COLOR32_MAGENTA = { 255,   0, 255, 255 };
    constexpr color32 CG_COLOR32_YELLOW  = { 255, 255,   0, 255 };
    
    constexpr float CG_FLOAT_PI = 3.14159274F;
    constexpr float CG_FLOAT_2PI = 2.0f * 3.14159274F;
    constexpr float CG_FLOAT_DEG2RAD = 0.0174532924F;
    constexpr float CG_FLOAT_RAD2DEG = 57.29578F;
    
    struct FrustumPlanes
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
        ushort ToNativeEnum(CG_TYPE type);
        CG_TYPE FromUniformType(ushort type);
        CG_TYPE FromUniformString(const char* string);
        std::string ToString(CG_TYPE type);
        CG_TYPE FromString(const char* string);
    };
    
    namespace Functions
    {
        float4x4 GetMatrixTRS(const float3& position, const quaternion& rotation, const float3& scale);
        float4x4 GetMatrixTRS(const float3& position, const float3& euler, const float3& scale);
        float4x4 GetMatrixInvTRS(const float3& position, const quaternion& rotation, const float3& scale);
        float4x4 GetMatrixInvTRS(const float3& position, const float3& euler, const float3& scale);
        float4x4 GetMatrixTR(const float3& position, const quaternion& rotation);
        float4x4 GetPerspective(float fov, float aspect, float nearClip, float farClip);
        float4x4 GetOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
        float4x4 GetOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
        inline float GetZNearFromProj(const float4x4& matrix) { return -matrix[3][2] / (matrix[2][2] + 1.0f); }
        inline float GetZFarFromProj(const float4x4& matrix) { return -matrix[3][2] / (matrix[2][2] - 1.0f); }
        inline float GetSizeOnScreen(float depth, float sizePerDepth, float radius) { return radius / (sizePerDepth * depth); }
        float4x4 GetOffsetPerspective(float left, float right, float bottom, float top, float fovy, float aspect, float zNear, float zFar);
        float4x4 GetPerspectiveSubdivision(int index, const int3& gridSize, float fovy, float aspect, float znear, float zfar);
        float4x4 GetFrustumBoundingOrthoMatrix(const float4x4& worldToLocal, const float4x4& inverseViewProjection, const float3& paddingLD, const float3& paddingRU, float* outZnear, float* outZFar);
        float GetShadowCascadeMatrices(const float4x4& worldToLocal, const float4x4& inverseViewProjection, const float* zPlanes, float zPadding, uint count, float4x4* matrices);

        inline color HexToRGB(uint hex) { return color((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF, 255.0f) / 255.0f; }
        color HueToRGB(float hue);
        void GetCascadeDepths(float znear, float zfar, float linearity, float* cascades, uint count);
        inline float CascadeDepth(float znear, float zfar, float linearity, float interpolant) { return linearity * (znear * powf(zfar / znear, interpolant)) + (1.0f - linearity) * (znear + (zfar - znear) * interpolant); }
        inline float Cot(float value) { return cos(value) / sin(value); }
        inline float RandomFloat() { return (float)rand() / (float)RAND_MAX; }
        inline float3 RandomFloat3() { return float3(RandomFloat(), RandomFloat(), RandomFloat()); }
        inline float RandomRangeFloat(float min, float max) { return min + (RandomFloat() * (max - min)); }
        inline float3 RandomRangeFloat3(const float3& min, const float3& max) { return float3(RandomRangeFloat(min.x, max.x), RandomRangeFloat(min.y, max.y), RandomRangeFloat(min.z, max.z)); }
        inline float3 RandomEuler() { return float3(RandomRangeFloat(-360.0f, 360.0f), RandomRangeFloat(-360.0f, 360.0f), RandomRangeFloat(-360.0f, 360.0f)); }
        size_t GetNextExponentialSize(size_t start, size_t min);
        inline uint GetMaxMipLevelPow2(uint resolution) { return glm::log2(resolution); }
        inline uint GetMaxMipLevelPow2(uint2 resolution) { return glm::log2(glm::compMin(resolution)); }
        inline uint GetMaxMipLevelPow2(uint3 resolution) { return glm::log2(glm::compMin(resolution)); }
        uint GetMaxMipLevel(uint resolution);
        inline uint GetMaxMipLevel(uint2 resolution) { return GetMaxMipLevel(glm::compMin(resolution)); }
        inline uint GetMaxMipLevel(uint3 resolution) { return GetMaxMipLevel(glm::compMin(resolution)); }
        uint ByteArrayHash(const void* data, size_t count);
        
        // Color grading math
        color NormalizeColor(const color& color);
        // CIE xy chromaticity to CAT02 LMS.
        // http://en.wikipedia.org/wiki/LMS_color_space#CAT02
        float3 CIExyToLMS(float x, float y);
        float4 GetWhiteBalance(float temperatureShift, float tint);
        void GenerateLiftGammaGain(const color& shadows, const color& midtones, const color& highlights, color* outLift, color* outGamma, color* outGain);
        // An analytical model of chromaticity of the standard illuminant, by Judd et al.
        // http://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_series_D
        // Slightly modifed to adjust it with the D65 white point (x=0.31271, y=0.32902).
        inline float StandardIlluminantY(float x) { return 2.87f * x - 3.0f * x * x - 0.27509507f; }
        inline float Luminance(const color& color) { return glm::dot(float3(0.22f, 0.707f, 0.071f), float3(color.rgb)); }
        inline float LinearToPerceptual(const color& color) { return glm::log(glm::max(Luminance(color), 0.001f)); }
        inline float PerceptualToLinear(float value) { return glm::exp(value); }

        void NormalizePlane(float4* plane);
        void ExtractFrustrumPlanes(const float4x4 viewprojection, FrustumPlanes* frustrum, bool normalize);
    
        float PlaneDistanceToAABB(const float4& plane, const BoundingBox& aabb);
        
        inline float PlaneDistanceToPoint(const float4& plane, const float3& point) { return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w; }
    
        inline float3 IntesectPlanes3(const float4& p1, const float4& p2, const float4& p3)
        {
            float3 n1 = p1.xyz, n2 = p2.xyz, n3 = p3.xyz;
            return ((-p1.w * glm::cross(n2, n3)) + (-p2.w * glm::cross(n3, n1)) + (-p3.w * glm::cross(n1, n2))) / (glm::dot(n1, glm::cross(n2, n3)));
        }
    
        inline BoundingBox CreateBoundsMinMax(const float3& min, const float3& max) { return BoundingBox(min, max); }
        inline BoundingBox CreateBoundsCenterExtents(const float3& center, const float3& extents) { return BoundingBox(center - extents, center + extents); }
    
        bool IntersectPlanesAABB(const float4* planes, int planeCount, const BoundingBox& aabb);
        bool IntersectAABB(const BoundingBox& a, const BoundingBox& b);
        bool IntersectSphere(const float3& center, float radius, const BoundingBox& b);
        void BoundsEncapsulate(BoundingBox* bounds, const BoundingBox& other);
        int BoundsLongestAxis(const BoundingBox& bounds);
        int BoundsShortestAxis(const BoundingBox& bounds);
        void BoundsSplit(const BoundingBox& bounds, int axis, BoundingBox* out0, BoundingBox* out1);
        bool BoundsContains(const BoundingBox& bounds, const float3& point);
        BoundingBox BoundsTransform(const float4x4& matrix, const BoundingBox& bounds);
        BoundingBox GetInverseFrustumBounds(const float4x4& inverseMatrix);
        BoundingBox GetInverseFrustumBounds(const float4x4& inverseMatrix, float lznear, float lzfar);
        BoundingBox GetInverseFrustumBounds(const float4x4& worldToLocal, const float4x4& inverseMatrix);
    };
}