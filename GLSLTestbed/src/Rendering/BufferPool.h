#pragma once
#include "Rendering/Objects/Material.h"
#include "Rendering/Objects/Mesh.h"

namespace PK::Rendering
{
    using namespace PK::Utilities;
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    class BufferPool
    {
        public:
            Ref<ComputeBuffer>& GetBuffer(size_t size);
            void Reset();

        private:
            std::vector<Ref<ComputeBuffer>> m_buffers;
            BufferLayout m_layout;
            uint m_usedCount;
    };
}