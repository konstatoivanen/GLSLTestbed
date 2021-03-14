#pragma once
#include "Core/EntityDatabase.h"
#include "Context/Structs/Components.h"

namespace PKECS::EntityViews
{
    struct BaseRenderable : public IEntityView
    {
        Components::Bounds* bounds;
        Components::RenderableHandle* handle;
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
}