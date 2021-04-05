#pragma once
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Material.h"
#include <hlslmath.h>

namespace PK::ECS::Components
{
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    struct Transform
    {
        float3 position = CG_FLOAT3_ZERO;
        quaternion rotation = CG_QUATERNION_IDENTITY;
        float3 scale = CG_FLOAT3_ONE;
        float4x4 localToWorld = CG_FLOAT4X4_IDENTITY;
        float4x4 worldToLocal = CG_FLOAT4X4_IDENTITY;

        float4x4 GetLocalToWorld() const { return Functions::GetMatrixTRS(position, rotation, scale); }
        float4x4 GetWorldToLocal() const { return Functions::GetMatrixInvTRS(position, rotation, scale); }

        virtual ~Transform() = 0 {}
    };
    
    struct Bounds
    {
        BoundingBox localAABB;
        BoundingBox worldAABB;
        virtual ~Bounds() = 0 {}
    };
    
    enum class RenderHandleFlags : ushort
    {
        Renderer = 1 << 0,
        Light = 1 << 1,
        Static = 1 << 2,
        Dynamic = 1 << 3,
        ShadowCaster = 1 << 4,
        LightPoint = 1 << 5,
        LightSpot = 1 << 6,
        LightDirectional = 1 << 7
    };

    inline RenderHandleFlags operator|(RenderHandleFlags a, RenderHandleFlags b)
    {
        return static_cast<RenderHandleFlags>(static_cast<ushort>(a) | static_cast<ushort>(b));
    }

    struct RenderableHandle
    {
        bool isVisible = false;
        bool isCullable = true;
        RenderHandleFlags flags = RenderHandleFlags::Renderer;
        float viewSize = 1.0f;
        virtual ~RenderableHandle() = 0 {}
    };
    
    struct MeshReference
    {
        Weak<Mesh> sharedMesh;
        virtual ~MeshReference() = 0 {}
    };
    
    struct Materials
    {
        std::vector<Weak<Material>> sharedMaterials;
        virtual ~Materials() = 0 {}
    };
    
    struct PointLight
    {
        color color = CG_COLOR_WHITE;
        float radius = 1.0f;
        virtual ~PointLight() = 0 {}
    };
}