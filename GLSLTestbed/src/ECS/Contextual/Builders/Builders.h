#pragma once
#include "Rendering/Objects/Mesh.h"
#include "Rendering/Objects/Material.h"
#include "ECS/Contextual/Implementers/Implementers.h"
#include <hlslmath.h>

namespace PK::ECS::Builders
{
    using namespace PK::ECS::Implementers;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    void InitializeLightValues(LightImplementer* implementer, const color& color, LightType lightType, LightCookie cookie, bool castShadows, float angle, float radius = -1);
}