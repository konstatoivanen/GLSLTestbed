#pragma once
#include "Core/Sequencer.h"
#include "Core/IService.h"
#include "Rendering/Objects/Buffer.h"
#include "Rendering/Objects/Shader.h"
#include <hlslmath.h>

class GizmoRenderer : public IService, public PKECS::ISimpleStep
{
    struct GizmoVertex
    {
        float3 position;
        float  zoffset;
        float4 color;
    };

    public:
        GizmoRenderer(PKECS::Sequencer* sequencer, AssetDatabase* assetDatabase);

        void Step(int condition) override;

        void DrawWireBounds(const BoundingBox& aabb);
        void DrawWireBox(const float3& origin, const float3& size);
        void DrawLine(const float3& start, const float3& end);
        void DrawRay(const float3& origin, const float3& vector);
        void DrawFrustrum(const float4x4 matrix);
        void SetColor(const color& color);
        void SetMatrix(const float4x4& matrix);

    private:
        GizmoVertex* ReserveVertices(uint count);

        PKECS::Sequencer* m_sequencer;
        FrustrumPlanes m_frustrumPlanes;
        Ref<ComputeBuffer> m_vertexBuffer;
        Weak<Shader> m_gizmoShader;
        std::vector<GizmoVertex> m_vertices;
        uint m_vertexCount = 0;
        color m_color = CG_COLOR_WHITE;
};

