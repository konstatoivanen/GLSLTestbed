#pragma once
#include "ECS/EntityDatabase.h"
#include "ECS/Contextual/Components/Components.h"

namespace PK::ECS::Implementers
{
    struct MeshRenderableImplementer : public IImplementer,
        public Components::Transform,
        public Components::Bounds,
        public Components::RenderableHandle,
        public Components::MeshReference,
        public Components::Materials
    {
    };

    struct LightImplementer : public IImplementer,
        public Components::Transform,
        public Components::Bounds,
        public Components::RenderableHandle,
        public Components::Light
    {
    };
}