#pragma once
#include "Core/EntityDatabase.h"
#include "Context/Structs/Components.h"

namespace PKECS::Implementers
{
    struct MeshRenderableImplementer : public IImplementer,
        public Components::Transform,
        public Components::Bounds,
        public Components::RenderableHandle,
        public Components::MeshReference,
        public Components::Materials
    {
    };

    struct PointLightImplementer : public IImplementer,
        public Components::Transform,
        public Components::Bounds,
        public Components::RenderableHandle,
        public Components::PointLight
    {
    };
}