#pragma once
#include "Core/BufferView.h"
#include "ECS/EntityDatabase.h"
#include <vector>
#include <hlslmath.h>

namespace PK::Rendering::Culling
{
    using namespace PK::Math;

    enum class CullingGroup : ushort
    {
        CameraFrustum,
        ShadowFrustum,
    };

    typedef void (*OnVisibleItem)(ECS::EntityDatabase*, ECS::EGID, float depth, void*);

    typedef void (*OnVisibleItemMulti)(ECS::EntityDatabase*, ECS::EGID, uint clipIndex, float depth, void*);

    class VisibilityCache
    {
        private:
            struct VisibilityList
            {
                std::vector<uint> list;
                size_t count = 0;
            };

        public:
            void AddItem(CullingGroup group, ushort type, uint item);
            
            void Reset();

            Core::BufferView<uint> GetList(CullingGroup group, ushort type)
            {
                auto key = (uint)type << 16 | (uint)group & 0xFFFF;

                if (m_visibilityLists.count(key))
                {
                    auto& element = m_visibilityLists.at(key);
                    return { element.list.data(), element.count };
                }

                return { nullptr, 0 };
            }

        private:            
            std::map<uint, VisibilityList> m_visibilityLists;
    };

    void ExecuteOnVisibleItemsCubeFaces(PK::ECS::EntityDatabase* entityDb, const BoundingBox& aabb, ushort typeMask, OnVisibleItemMulti onvisible, void* context);

    void ExecuteOnVisibleItemsFrustum(PK::ECS::EntityDatabase* entityDb, const float4x4& matrix, ushort typeMask, OnVisibleItem onvisible, void* context);

    void ExecuteOnVisibleItemsAABB(PK::ECS::EntityDatabase* entityDb, const BoundingBox& aabb, ushort typeMask, OnVisibleItem onvisible, void* context);

    void ExecuteOnVisibleItemsSphere(PK::ECS::EntityDatabase* entityDb, const float3& center, float radius, ushort typeMask, OnVisibleItem onvisible, void* context);

    void BuildVisibilityCacheFrustum(PK::ECS::EntityDatabase* entityDb, VisibilityCache* cache, const float4x4& matrix, CullingGroup group, ushort typeMask);
    
    void BuildVisibilityCacheAABB(PK::ECS::EntityDatabase* entityDb, VisibilityCache* cache, const BoundingBox& aabb, CullingGroup group, ushort typeMask);

    void ResetEntityVisibilities(PK::ECS::EntityDatabase* entityDb);
}