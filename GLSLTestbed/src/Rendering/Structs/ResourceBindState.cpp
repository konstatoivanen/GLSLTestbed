#include "PrecompiledHeader.h"
#include "ResourceBindState.h"
#include "Utilities/Ref.h"
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

        glGetIntegerv(GL_MAX_IMAGE_UNITS, &count);
        m_bindings[CG_TYPE::IMAGE_PARAMS] = std::vector<GraphicsID>(count);

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

    void ResourceBindState::BindImages(ushort location, const ImageBindDescriptor* imageBindings, ushort count)
    {
        auto bindings = GetBindings(CG_TYPE::IMAGE_PARAMS, location, count);

        if (count == 1)
        {
            // @TODO Could result in some collisions but whatever
            auto hash = Functions::ByteArrayHash(imageBindings, sizeof(ImageBindDescriptor));

            if (bindings[0] != hash)
            {
                bindings[0] = hash;
                glBindImageTexture(location, imageBindings->graphicsId, imageBindings->level, imageBindings->layered, imageBindings->layer, imageBindings->access, imageBindings->format);
            }

            return;
        }

        for (ushort i = 0; i < count; ++i)
        {
            // @TODO Could result in some collisions but whatever
            auto hash = Functions::ByteArrayHash(imageBindings + i, sizeof(ImageBindDescriptor));

            if (bindings[i] == hash)
            {
                continue;
            }

            auto ids = PK_STACK_ALLOC(GraphicsID, count);
            auto hashes = PK_STACK_ALLOC(uint, count);

            for (auto j = 0; j < count; ++j)
            {
                ids[j] = imageBindings[j].graphicsId;
                hashes[j] = Functions::ByteArrayHash(imageBindings + j, sizeof(ImageBindDescriptor));
            }

            memcpy(bindings, hashes, sizeof(uint) * count);
            glBindImageTextures(location, count, ids);
            break;
        }
    }
    
    void ResourceBindState::BindBuffers(CG_TYPE type, ushort location, const GraphicsID* graphicsIds, ushort count)
    {
        auto bindings = GetBindings(type, location, count);
        auto nativeType = Convert::ToNativeEnum(type);
    
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