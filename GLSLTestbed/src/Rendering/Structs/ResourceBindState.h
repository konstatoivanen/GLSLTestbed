#pragma once
#include "Rendering/Objects/GraphicsObject.h"
#include <hlslmath.h>

class ResourceBindState
{
    public:
        ResourceBindState();
        void BindTextures(ushort location, const GraphicsID* graphicsId, ushort count);
        void BindBuffers(ushort type, ushort location, const GraphicsID* graphicsId, ushort count);

    private:
        GraphicsID* GetBindings(ushort type, ushort location, ushort count);
        std::map<ushort, std::vector<GraphicsID>> m_bindings;
};