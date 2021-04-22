#pragma once
#include "Core/IService.h"
#include "ECS/Sequencer.h"
#include "Rendering/Objects/Buffer.h"
#include "Rendering/Objects/Shader.h"
#include <hlslmath.h>

namespace PK::Rendering
{
    using namespace Utilities;
    using namespace Objects;

    class GizmoRenderer : public IService, public ECS::ISimpleStep
    {
        struct GizmoVertex
        {
            float3 position;
            float  zoffset;
            float4 color;
        };
    
        public:
            GizmoRenderer(ECS::Sequencer* sequencer, AssetDatabase* assetDatabase, bool enabled);
    
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
    
            ECS::Sequencer* m_sequencer;
            FrustrumPlanes m_frustrumPlanes;
            Ref<ComputeBuffer> m_vertexBuffer;
            Shader* m_gizmoShader;
            std::vector<GizmoVertex> m_vertices;
            uint m_vertexCount = 0;
            color m_color = CG_COLOR_WHITE;
            bool m_enabled = true;
    };
}