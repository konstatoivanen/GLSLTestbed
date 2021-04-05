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

    struct VisibilityList
    {
        std::vector<uint> list;
        size_t count = 0;
    };

    class VisibilityCache
    {
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

    void BuildVisibilityListFrustum(PK::ECS::EntityDatabase* entityDb, VisibilityList* list, const float4x4& matrix, ushort typeMask);

    void BuildVisibilityListAABB(PK::ECS::EntityDatabase* entityDb, VisibilityList* list, const BoundingBox& aabb, ushort typeMask);

    void BuildVisibilityListSphere(PK::ECS::EntityDatabase* entityDb, VisibilityList* list, const float3& center, float radius, ushort typeMask);

    void BuildVisibilityCacheFrustum(PK::ECS::EntityDatabase* entityDb, VisibilityCache* cache, const float4x4& matrix, CullingGroup group, ushort typeMask);
    
    void BuildVisibilityCacheAABB(PK::ECS::EntityDatabase* entityDb, VisibilityCache* cache, const BoundingBox& aabb, CullingGroup group, ushort typeMask);

    void ResetEntityVisibilities(PK::ECS::EntityDatabase* entityDb);
}