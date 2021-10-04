#pragma once
#include "Rendering/Objects/GraphicsObject.h"
#include "Rendering/Objects/Texture.h"
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
            void BindImages(ushort location, const ImageBindDescriptor* imageBindings, ushort count);
            void BindBuffers(PK_TYPE type, ushort location, const GraphicsID* graphicsId, ushort count);
            void BindMesh(GraphicsID graphicsId);
            void ResetBindStates();
        private:
            GraphicsID* GetBindings(PK_TYPE type, ushort location, ushort count);
            std::map<PK_TYPE, std::vector<GraphicsID>> m_bindings;
    };
}