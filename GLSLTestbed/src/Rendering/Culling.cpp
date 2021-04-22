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

	void Culling::ExecuteOnVisibleItemsCubeFaces(PK::ECS::EntityDatabase* entityDb, const BoundingBox& aabb, ushort typeMask, OnVisibleItemMulti onvisible, void* context)
	{
		const float3 planeNormals[] = { {-1,1,0}, {1,1,0}, {1,0,1}, {1,0,-1}, {0,1,1}, {0,-1,1} };
		const float3 absPlaneNormals[] = { {1,1,0}, {1,1,0}, {1,0,1}, {1,0,1}, {0,1,1}, {0,1,1} };

		auto aabbcenter = aabb.GetCenter();

		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (((ushort)cullable->handle->flags & typeMask) != typeMask || !Functions::IntersectAABB(aabb, cullable->bounds->worldAABB))
			{
				continue;
			}

			auto center = cullable->bounds->worldAABB.GetCenter() - aabbcenter;
			auto extents = cullable->bounds->worldAABB.GetExtents();

			bool rp[6];
			bool rn[6];
			bool vis[6];

			// Source: https://newq.net/dl/pub/s2015_shadows.pdf
			for (uint j = 0; j < 6; ++j)
			{
				auto dist = glm::dot(center, planeNormals[j]);
				auto radius = glm::dot(extents, absPlaneNormals[j]);
				rp[j] = dist > -radius;
				rn[j] = dist < radius;
			}

			vis[0] = rn[0] && rp[1] && rp[2] && rp[3] && cullable->bounds->worldAABB.max.x > aabbcenter.x;
			vis[1] = rp[0] && rn[1] && rn[2] && rn[3] && cullable->bounds->worldAABB.min.x < aabbcenter.x;

			vis[2] = rp[0] && rp[1] && rp[4] && rn[5] && cullable->bounds->worldAABB.max.y > aabbcenter.y;
			vis[3] = rn[0] && rn[1] && rn[4] && rp[5] && cullable->bounds->worldAABB.min.y < aabbcenter.y;

			vis[4] = rp[2] && rn[3] && rp[4] && rp[5] && cullable->bounds->worldAABB.max.z > aabbcenter.z;
			vis[5] = rn[2] && rp[3] && rn[4] && rn[5] && cullable->bounds->worldAABB.min.z < aabbcenter.z;

			for (uint j = 0; j < 6; ++j)
			{
				auto isVisible = !cullable->handle->isCullable || vis[j];
				cullable->handle->isVisible |= isVisible;

				if (isVisible)
				{
					onvisible(entityDb, cullable->GID, j, 0.0f, context);
				}
			}
		}
	}

	void Culling::ExecuteOnVisibleItemsFrustum(PK::ECS::EntityDatabase* entityDb, const float4x4& matrix, ushort typeMask, OnVisibleItemMulti onvisible, void* context)
	{
		FrustrumPlanes frustum;
		Functions::ExtractFrustrumPlanes(matrix, &frustum, true);

		auto cullables = entityDb->Query<ECS::EntityViews::BaseRenderable>((int)ECS::ENTITY_GROUPS::ACTIVE);

		for (auto i = 0; i < cullables.count; ++i)
		{
			auto cullable = &cullables[i];

			if (((ushort)cullable->handle->flags & typeMask) != typeMask)
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectPlanesAABB(frustum.planes, 6, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				onvisible(entityDb, cullable->GID, 0u, Functions::PlaneDistanceToAABB(frustum.planes[4], cullable->bounds->worldAABB), context);
			}
		}
	}

	void Culling::ExecuteOnVisibleItemsAABB(PK::ECS::EntityDatabase* entityDb, const BoundingBox& aabb, ushort typeMask, OnVisibleItem onvisible, void* context)
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
				onvisible(entityDb, cullable->GID, 0.0f, context);
			}
		}
	}

	void Culling::ExecuteOnVisibleItemsSphere(PK::ECS::EntityDatabase* entityDb, const float3& center, float radius, ushort typeMask, OnVisibleItem onvisible, void* context)
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
				onvisible(entityDb, cullable->GID, 0.0f, context);
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
			auto maskedFlags = (ushort)cullable->handle->flags & typeMask;

			if (!maskedFlags)
			{
				continue;
			}

			auto isVisible = !cullable->handle->isCullable || Functions::IntersectPlanesAABB(frustrum.planes, 6, cullable->bounds->worldAABB);
			cullable->handle->isVisible |= isVisible;

			if (isVisible)
			{
				cache->AddItem(group, maskedFlags, cullable->GID.entityID());
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