#include "PrecompiledHeader.h"
#include "Culling.h"
#include "ECS/Contextual/EntityViews/EntityViews.h"
#include "Utilities/Utilities.h"

namespace PK::Rendering::Culling
{
	void VisibilityCache::AddItem(CullingGroup group, ushort type, uint item)
	{
		auto key = ((uint)type << 16) | ((uint)group & 0xFFFF);
		auto& vis = m_visibilityLists[key];
		Utilities::PushVectorElementRef(vis.list, &vis.count, item);
	}

	void VisibilityCache::Reset()
	{
		for (auto& kv : m_visibilityLists)
		{
			kv.second.count = 0;
		}
	}

	void Culling::BuildVisibilityListFrustum(PK::ECS::EntityDatabase* entityDb, VisibilityList* list, const float4x4& matrix, ushort typeMask)
	{
		FrustrumPlanes frustrum;
		Functions::ExtractFrustrumPlanes(matrix, &frustrum, true);

		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (!((ushort)cullable->handle->flags & typeMask))
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectPlanesAABB(frustrum.planes, 6, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				Utilities::PushVectorElement(list->list, &list->count, cullable->GID.entityID());
			}
		}
	}

	void Culling::BuildVisibilityListAABB(PK::ECS::EntityDatabase* entityDb, VisibilityList* list, const BoundingBox& aabb, ushort typeMask)
	{
		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (!((ushort)cullable->handle->flags & typeMask))
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectAABB(aabb, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				Utilities::PushVectorElement(list->list, &list->count, cullable->GID.entityID());
			}
		}
	}

	void Culling::BuildVisibilityListSphere(PK::ECS::EntityDatabase* entityDb, VisibilityList* list, const float3& center, float radius, ushort typeMask)
	{
		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (!((ushort)cullable->handle->flags & typeMask))
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectSphere(center, radius, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				Utilities::PushVectorElement(list->list, &list->count, cullable->GID.entityID());
			}
		}
	}

	void Culling::BuildVisibilityCacheFrustum(PK::ECS::EntityDatabase* entityDb, VisibilityCache* cache, const float4x4& matrix, CullingGroup group, ushort typeMask)
	{
		FrustrumPlanes frustrum;
		Functions::ExtractFrustrumPlanes(matrix, &frustrum, true);

		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (!((ushort)cullable->handle->flags & typeMask))
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectPlanesAABB(frustrum.planes, 6, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				cache->AddItem(group, (ushort)cullable->handle->flags, cullable->GID.entityID());
			}
		}
	}

	void Culling::BuildVisibilityCacheAABB(PK::ECS::EntityDatabase* entityDb, VisibilityCache* cache, const BoundingBox& aabb, CullingGroup group, ushort typeMask)
	{
		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (!((ushort)cullable->handle->flags & typeMask))
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectAABB(aabb, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				cache->AddItem(group, (ushort)cullable->handle->flags, cullable->GID.entityID());
			}
		}
	}

	void Culling::ResetEntityVisibilities(PK::ECS::EntityDatabase* entityDb)
	{
		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			cullables[i].handle->isVisible = false;
		}
	}
}