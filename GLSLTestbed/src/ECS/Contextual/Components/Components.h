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

        inline float4x4 GetLocalToWorld() const { return Functions::GetMatrixTRS(position, rotation, scale); }
        inline float4x4 GetWorldToLocal() const { return Functions::GetMatrixInvTRS(position, rotation, scale); }

        virtual ~Transform() = default;
    };
    
    struct Bounds
    {
        BoundingBox localAABB;
        BoundingBox worldAABB;
        virtual ~Bounds() = default;
    };
    
    enum class RenderHandleFlags : ushort
    {
        Renderer = 1 << 0,
        Light = 1 << 1,
        Static = 1 << 2,
        ShadowCaster = 1 << 3,
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
        virtual ~RenderableHandle() = default;
    };
    
    struct MeshReference
    {
        Mesh* sharedMesh = nullptr;
        virtual ~MeshReference() = default;
    };
    
    struct Materials
    {
        std::vector<Material*> sharedMaterials;
        virtual ~Materials() = default;
    };
    
    struct Light
    {
        color color = CG_COLOR_WHITE;
        float radius = 1.0f;
        float angle = 45.0f;
        bool castShadows = true;
        uint linearIndex = 0;
        uint shadowmapIndex = 0;
        uint projectionIndex = 0;
        LightCookie cookie = LightCookie::NoCookie;
        LightType lightType = LightType::Point;
        virtual ~Light() = default;
    };
}