#include "PrecompiledHeader.h"
#include "EngineUpdateTransforms.h"
#include "Context/Structs/EntityViews.h"

EngineUpdateTransforms::EngineUpdateTransforms(PKECS::EntityDatabase* entityDb)
{
    m_entityDb = entityDb;
}

void EngineUpdateTransforms::Step(int condition)
{
    auto views = m_entityDb->Query<PKECS::EntityViews::TransformView>((int)PKECS::ENTITY_GROUPS::ACTIVE);

    for (auto i = 0; i < views.count; ++i)
    {
        auto view = &views[i];
        view->transform->localToWorld = view->transform->GetLocalToWorld();
        view->bounds->worldAABB = CGMath::BoundsTransform(view->transform->localToWorld, view->bounds->localAABB);
    }
}
