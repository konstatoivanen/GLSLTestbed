#pragma once
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Material.h"
#include <hlslmath.h>

namespace PKECS::Components
{
    struct Transform
    {
        float3 position = CG_FLOAT3_ZERO;
        quaternion rotation = CG_QUATERNION_IDENTITY;
        float3 scale = CG_FLOAT3_ONE;

        float4x4 GetLocalToWorld() const { return CGMath::GetMatrixTRS(position, rotation, scale); }

        virtual ~Transform() = 0 {}
    };
    
    struct Bounds
    {
        BoundingBox aabb;
        virtual ~Bounds() = 0 {}
    };
    
    enum class RenderHandleType
    {
        MeshRenderer,
        SkinnedMeshRenderer,
        PointLight
    };

    struct RenderableHandle
    {
        bool isVisible;
        bool isCullable = true;
        RenderHandleType type;
        float viewSize;
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
        color color;
        float radius;
        virtual ~PointLight() = 0 {}
    };
}