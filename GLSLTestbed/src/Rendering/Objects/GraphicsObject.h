#pragma once
#include "Core/NoCopy.h"

namespace PK::Rendering::Objects
{
    typedef uint32_t GraphicsID;
    
    class GraphicsObject : public PK::Core::NoCopy
    {
        public:
            virtual ~GraphicsObject() = default;
    
            GraphicsID GetGraphicsID() const { return m_graphicsId; }
            bool IsValid() const { return m_graphicsId != 0; }
    
            bool operator==(const GraphicsObject& other) const
            {
                return m_graphicsId == ((GraphicsObject&)other).m_graphicsId;
            }
    
        protected:
            GraphicsID m_graphicsId = 0;
    };
}