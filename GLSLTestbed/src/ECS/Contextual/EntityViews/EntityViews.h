#pragma once
#include "ECS/EntityDatabase.h"
#include "ECS/Contextual/Components/Components.h"

namespace PK::ECS::EntityViews
{
    struct BaseRenderable : public IEntityView
    {
        Components::Bounds* bounds;
        Components::RenderableHandle* handle;
    };

    struct TransformView : public IEntityView
    {
        Components::Transform* transform;
        Components::Bounds* bounds;
    };
    
    struct MeshRenderable : public IEntityView
    {
        Components::Transform* transform;
        Components::MeshReference* mesh;
        Components::Materials *materials;
    };

    struct PointLightRenderable : public IEntityView
    {
        Components::Transform* transform;
        Components::PointLight* pointLight;
    };

    struct LightSphere : public IEntityView
    {
        Components::Transform* transformMesh;
        Components::Transform* transformLight;
    };
}