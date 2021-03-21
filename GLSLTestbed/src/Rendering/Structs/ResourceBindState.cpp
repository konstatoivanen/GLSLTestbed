#include "PrecompiledHeader.h"
#include "ResourceBindState.h"
#include "Utilities/Log.h"

namespace PK::Rendering::Structs
{
    using namespace PK::Rendering::Objects;

    ResourceBindState::ResourceBindState()
    {
        GLint count;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &count);
        m_bindings[CG_TYPE::TEXTURE] = std::vector<GraphicsID>(count);
    
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &count);
        m_bindings[CG_TYPE::CONSTANT_BUFFER] = std::vector<GraphicsID>(count);
    
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &count);
        m_bindings[CG_TYPE::COMPUTE_BUFFER] = std::vector<GraphicsID>(count);
    
        m_bindings[CG_TYPE::VERTEX_ARRAY] = std::vector<GraphicsID>(1);
    }
    
    void ResourceBindState::BindTextures(ushort location, const GraphicsID* graphicsIds, ushort count)
    {
        auto bindings = GetBindings(CG_TYPE::TEXTURE, location, count);
    
        if (count == 1)
        {
            if (bindings[0] != graphicsIds[0])
            {
                bindings[0] = graphicsIds[0];
                glBindTextureUnit(location, graphicsIds[0]);
            }
    
            return;
        }
    
        for (ushort i = 0; i < count; ++i)
        {
            if (bindings[i] == graphicsIds[i])
            {
                continue;
            }
    
            memcpy(bindings, graphicsIds, sizeof(GraphicsID) * count);
            glBindTextures(location, count, graphicsIds);
            break;
        }
    }
    
    void ResourceBindState::BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsIds, ushort count)
    {
        auto bindings = GetBindings(type, location, count);
        auto nativeType = Convert::NativeEnum(type);
    
        for (ushort i = 0; i < count; ++i)
        {
            if (bindings[i] == graphicsIds[i])
            {
                continue;
            }
    
            bindings[i] = graphicsIds[i];
            glBindBufferBase(nativeType, location + i, graphicsIds[i]);
        }
    }
    
    void ResourceBindState::BindMesh(GraphicsID graphicsId)
    {
        auto* binding = m_bindings.at(CG_TYPE::VERTEX_ARRAY).data();
    
        if (binding[0] != graphicsId)
        {
            binding[0] = graphicsId;
            glBindVertexArray(graphicsId);
        }
    }
    
    void ResourceBindState::ResetBindStates()
    {
        for (auto& i : m_bindings)
        {
            std::fill(i.second.begin(), i.second.end(), 0);
        }
    }
    
    GraphicsID* ResourceBindState::GetBindings(CG_TYPE type, ushort location, ushort count)
    {
        auto& bindings = m_bindings.at(type);
        auto* bbuff = bindings.data();
        PK_CORE_ASSERT((ushort)bindings.size() > (location + count), "MAXIMUM BUFFER BINDING INDEX EXCEEDED!");
        return bbuff + location;
    }
}