#pragma once
#include "Core/BufferView.h"
#include "ECS/EntityDatabase.h"
#include <vector>
#include <hlslmath.h>

namespace PK::Rendering
{
    using namespace PK::Math;

    class FrustumCuller : PK::Core::NoCopy
    {
        struct CullingResults
        {
            std::vector<uint> list;
            size_t count = 0;
        };

        public:
            void Update(PK::ECS::EntityDatabase* entityDb, const float4x4& matrix);
            Core::BufferView<uint> GetCullingResults(uint type)
            {
                auto& element = m_visibilityLists[type];
                return { element.list.data(), element.count };
            }

        private:
            std::map<uint, CullingResults> m_visibilityLists;
    };
}