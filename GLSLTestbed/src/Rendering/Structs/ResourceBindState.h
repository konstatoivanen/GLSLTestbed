#pragma once
#include "Rendering/Objects/GraphicsObject.h"
#include <hlslmath.h>

namespace PK::Rendering::Structs
{
    using namespace PK::Rendering::Objects;
    using namespace PK::Math;

    class ResourceBindState
    {
        public:
            ResourceBindState();
            void BindTextures(ushort location, const GraphicsID* graphicsId, ushort count);
            void BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsId, ushort count);
            void BindMesh(GraphicsID graphicsId);
            void ResetBindStates();
        private:
            GraphicsID* GetBindings(CG_TYPE type, ushort location, ushort count);
            std::map<CG_TYPE, std::vector<GraphicsID>> m_bindings;
    };
}