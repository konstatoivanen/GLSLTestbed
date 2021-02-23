#include "PrecompiledHeader.h"
#include "Rendering/MeshUtility.h"

namespace MeshUtilities
{
    Ref<Mesh> GetBoxSimple(const float3& offset, const float3& extents)
    {
        float vertices[] =
        {
            offset.x - extents.x, offset.y - extents.y, offset.z - extents.z,
            offset.x + extents.x, offset.y - extents.y, offset.z - extents.z,
            offset.x + extents.x, offset.y + extents.y, offset.z - extents.z,
            offset.x - extents.x, offset.y + extents.y, offset.z - extents.z,
    
            offset.x - extents.x, offset.y + extents.y, offset.z + extents.z,
            offset.x + extents.x, offset.y + extents.y, offset.z + extents.z,
            offset.x + extents.x, offset.y - extents.y, offset.z + extents.z,
            offset.x - extents.x, offset.y - extents.y, offset.z + extents.z,
        };
    
    
        unsigned int indices[] =
        {
            // face front
            0, 2, 1, 0, 3, 2,
    
            // face top
            2, 3, 4, 2, 4, 5,
    
            // face right
            1, 2, 5, 1, 5, 6,
    
            // face left
            7, 0, 4, 0, 4, 3,
    
            // face back
            5, 4, 7, 5, 7, 6,
    
            // face bottom
            0, 6, 7, 0, 1, 6
        };
    
        BufferLayout layout = { {CG_TYPE_FLOAT3, "POSITION"} };
        return CreateRef<Mesh>(CreateRef<VertexBuffer>(vertices, 8, layout), CreateRef<IndexBuffer>(indices, 36));
    }
    
    Ref<Mesh> GetBox(const float3& offset, const float3& extents)
    {
        float3 p0 = { offset.x - extents.x, offset.y - extents.y, offset.z + extents.z };
        float3 p1 = { offset.x + extents.x, offset.y - extents.y, offset.z + extents.z };
        float3 p2 = { offset.x + extents.x, offset.y - extents.y, offset.z - extents.z };
        float3 p3 = { offset.x - extents.x, offset.y - extents.y, offset.z - extents.z };
        float3 p4 = { offset.x - extents.x, offset.y + extents.y, offset.z + extents.z };
        float3 p5 = { offset.x + extents.x, offset.y + extents.y, offset.z + extents.z };
        float3 p6 = { offset.x + extents.x, offset.y + extents.y, offset.z - extents.z };
        float3 p7 = { offset.x - extents.x, offset.y + extents.y, offset.z - extents.z };
    
        float3 up = CG_FLOAT3_UP;
        float3 down = CG_FLOAT3_DOWN;
        float3 front = CG_FLOAT3_FORWARD;
        float3 back = CG_FLOAT3_BACKWARD;
        float3 left = CG_FLOAT3_LEFT;
        float3 right = CG_FLOAT3_RIGHT;
    
        float2 uv00 = { 0.0f, 0.0f };
        float2 uv10 = { 1.0f, 0.0f };
        float2 uv01 = { 0.0f, 1.0f };
        float2 uv11 = { 1.0f, 1.0f };
    
        Vertex_Full vertices[] =
        {
            // Bottom
            { p0, down, uv11 },
            { p1, down, uv01 },
            { p2, down, uv00 },
            { p3, down, uv10 },
    
            // Left
            { p7, left, uv11 },
            { p4, left, uv01 },
            { p0, left, uv00 },
            { p3, left, uv10 },
    
            // Front
            { p4, front, uv11 },
            { p5, front, uv01 },
            { p1, front, uv00 },
            { p0, front, uv10 },
    
            // Back
            { p6, back, uv11 },
            { p7, back, uv01 },
            { p3, back, uv00 },
            { p2, back, uv10 },
    
            // Right
            { p5, right, uv11 },
            { p6, right, uv01 },
            { p2, right, uv00 },
            { p1, right, uv10 },
    
            // Top
            { p7, up, uv11 },
            { p6, up, uv01 },
            { p5, up, uv00 },
            { p4, up, uv10 }
        };
    
        unsigned int indices[] =
        {
            // Bottom
            3, 1, 0, 3, 2, 1,
    
            // Left
            3 + 4 * 1, 1 + 4 * 1, 0 + 4 * 1,
            3 + 4 * 1, 2 + 4 * 1, 1 + 4 * 1,
    
            // Front
            3 + 4 * 2, 1 + 4 * 2, 0 + 4 * 2,
            3 + 4 * 2, 2 + 4 * 2, 1 + 4 * 2,
    
            // Back
            3 + 4 * 3, 1 + 4 * 3, 0 + 4 * 3,
            3 + 4 * 3, 2 + 4 * 3, 1 + 4 * 3,
    
            // Right
            3 + 4 * 4, 1 + 4 * 4, 0 + 4 * 4,
            3 + 4 * 4, 2 + 4 * 4, 1 + 4 * 4,
    
            // Top
            3 + 4 * 5, 1 + 4 * 5, 0 + 4 * 5,
            3 + 4 * 5, 2 + 4 * 5, 1 + 4 * 5,
        };
    
        BufferLayout layout = { {CG_TYPE_FLOAT3, "POSITION"}, {CG_TYPE_FLOAT3, "NORMAL"}, {CG_TYPE_FLOAT2, "TEXCOORD0"} };
        return CreateRef<Mesh>(CreateRef<VertexBuffer>(reinterpret_cast<float*>(vertices), 24, layout), CreateRef<IndexBuffer>(indices, 36));
    }
    
    Ref<Mesh> GetQuad2D(const float2& min, const float2& max)
    {
        float vertices[] =
        {
             min.x, min.y,
             0.0,   0.0f,
             min.x, max.y,
             0.0f,  1.0f,
             max.x, max.y,
             1.0f,  1.0f,
             max.x, min.y,
             1.0f,  0.0f
        };
    
        unsigned int indices[] =
        {
            0,1,2,
            2,3,0
        };
    
        BufferLayout layout = { {CG_TYPE_FLOAT2, "POSITION"}, {CG_TYPE_FLOAT2, "TEXCOORD0"} };
        return CreateRef<Mesh>(CreateRef<VertexBuffer>(vertices, 4, layout), CreateRef<IndexBuffer>(indices, 6));
    }
    
    Ref<Mesh> GetQuad3D(const float2& min, const float2& max)
    {
        float vertices[] =
        {
             min.x, min.y, 0.0f,
             0.0,   0.0f,
             min.x, max.y, 0.0f,
             0.0f,  1.0f,
             max.x, max.y, 0.0f,
             1.0f,  1.0f,
             max.x, min.y, 0.0f,
             1.0f,  0.0f
        };
    
        unsigned int indices[] =
        {
            0,1,2,
            2,3,0
        };
    
        BufferLayout layout = { {CG_TYPE_FLOAT3, "POSITION"}, {CG_TYPE_FLOAT2, "TEXCOORD0"} };
        return CreateRef<Mesh>(CreateRef<VertexBuffer>(vertices, 4, layout), CreateRef<IndexBuffer>(indices, 6));
    }
}