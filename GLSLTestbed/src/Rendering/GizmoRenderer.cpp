#include "PrecompiledHeader.h"
#include "Core/Time.h"
#include "Rendering/GizmoRenderer.h"
#include "Rendering/Graphics.h"

GizmoRenderer::GizmoRenderer(PKECS::Sequencer* sequencer, AssetDatabase* assetDatabase)
{
    m_sequencer = sequencer;
    m_gizmoShader = assetDatabase->Find<Shader>("SH_WS_Gizmos");
    m_vertexBuffer = CreateRef<ComputeBuffer>(BufferLayout({ { CG_TYPE_FLOAT4, "POSITION", CG_TYPE_FLOAT4, "COLOR" } }), 32);
}

void GizmoRenderer::DrawWireBounds(const float3& center, const float3& extents)
{
    DrawWireBox(center - extents, extents * 2.0f);
}

void GizmoRenderer::DrawWireBox(const float3& origin, const float3& size)
{
    auto vertices = ReserveVertices(24);

    auto idx = 0;
    vertices[idx++] = { origin + float3(0, 0, 0), 0, m_color };
    vertices[idx++] = { origin + float3(0, size.y, 0), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, 0, 0), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, size.y, 0), 0, m_color };
    vertices[idx++] = { origin + float3(0, 0, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(0, size.y, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, 0, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, size.y, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(0, 0, 0), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, 0, 0), 0, m_color };
    vertices[idx++] = { origin + float3(0, size.y, 0), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, size.y, 0), 0, m_color };
    vertices[idx++] = { origin + float3(0, 0, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, 0, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(0, size.y, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, size.y, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(0, 0, 0), 0, m_color };
    vertices[idx++] = { origin + float3(0, 0, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, 0, 0), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, 0, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(0, size.y, 0), 0, m_color };
    vertices[idx++] = { origin + float3(0, size.y, size.z), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, size.y, 0), 0, m_color };
    vertices[idx++] = { origin + float3(size.x, size.y, size.z), 0, m_color };
}

void GizmoRenderer::DrawLine(const float3& start, const float3& end)
{
    auto vertices = ReserveVertices(2);
    vertices[0] = { start, 0, m_color };
    vertices[1] = { end, 0, m_color };
}

void GizmoRenderer::DrawRay(const float3& origin, const float3& vector)
{
    DrawLine(origin, origin + vector);
}

void GizmoRenderer::DrawFrustrum(const float4x4 matrix)
{
    float3 nearCorners[4];
    float3 farCorners[4];
    FrustrumPlanes frustrum;

    CGMath::ExtractFrustrumPlanes(matrix, &frustrum, true);

    auto temp = frustrum.planes[1];
    frustrum.planes[1] = frustrum.planes[2];
    frustrum.planes[2] = temp;

    for (auto i = 0; i < 4; ++i)
    {
        nearCorners[i] = CGMath::IntesectPlanes3(frustrum.planes[4], frustrum.planes[i], frustrum.planes[(i + 1) % 4]);
        farCorners[i] = CGMath::IntesectPlanes3(frustrum.planes[5], frustrum.planes[i], frustrum.planes[(i + 1) % 4]);
    }

    auto vertices = ReserveVertices(24);

    for (auto i = 0; i < 4; ++i)
    {
        vertices[i * 6 + 0] = { nearCorners[i], 0, m_color };
        vertices[i * 6 + 1] = { nearCorners[(i + 1) % 4], 0, m_color };

        vertices[i * 6 + 2] = { farCorners[i], 0, m_color };
        vertices[i * 6 + 3] = { farCorners[(i + 1) % 4], 0, m_color };

        vertices[i * 6 + 4] = { nearCorners[i], 0, m_color };
        vertices[i * 6 + 5] = { farCorners[i], 0, m_color };
    }
}

void GizmoRenderer::SetColor(const color& color)
{
    m_color = color;
}

void GizmoRenderer::SetMatrix(const float4x4& matrix)
{
    Graphics::SetModelMatrix(matrix);
}

GizmoRenderer::GizmoVertex* GizmoRenderer::ReserveVertices(uint count)
{
    auto newCount = m_vertexCount + count;

    CGMath::ValidateVectorSize(m_vertices, newCount);

    auto ptr = m_vertices.data() + m_vertexCount;
    m_vertexCount += count;
    return ptr;
}

void GizmoRenderer::Step(int condition)
{
    SetColor(CG_COLOR_WHITE);
    SetMatrix(CG_FLOAT4X4_IDENTITY);
    m_vertexCount = 0;

    m_sequencer->Next<GizmoRenderer>(this, this, 0);

    if (m_vertexCount <= 0)
    {
        return;
    }

    m_vertexBuffer->ValidateSize((uint)m_vertices.size());
    m_vertexBuffer->MapBuffer(m_vertices.data(), m_vertices.size() * sizeof(GizmoVertex));

    Graphics::SetGlobalComputeBuffer(StringHashID::StringToID("pk_GizmoVertices"), m_vertexBuffer->GetGraphicsID());
    Graphics::DrawProcedural(m_gizmoShader.lock(), GL_LINES, 0, m_vertexCount);
}