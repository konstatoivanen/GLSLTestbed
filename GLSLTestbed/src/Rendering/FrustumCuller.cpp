#include "PrecompiledHeader.h"
#include "FrustumCuller.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"
#include "Utilities/Utilities.h"

namespace PK::Rendering
{
	void FrustumCuller::Update(ECS::EntityDatabase* entityDb, const float4x4& matrix)
	{
		FrustrumPlanes frustrum;
		Functions::ExtractFrustrumPlanes(matrix, &frustrum, true);
	
		for (auto& kv : m_visibilityLists)
		{
			kv.second.count = 0;
		}
	
		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);
	
		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];
			cullable->handle->isVisible = !cullable->handle->isCullable || Functions::IntersectPlanesAABB(frustrum.planes, 6, cullable->bounds->worldAABB);
	
			if (cullable->handle->isVisible)
			{
				auto& visibilityList = m_visibilityLists[(uint)cullable->handle->type];
				auto& list = visibilityList.list;
				auto& count = visibilityList.count;
				Utilities::ValidateVectorSize(list, count + 1);
				list[count++] = cullable->GID.entityID();
			}
		}
	}
}