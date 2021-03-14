#include "PrecompiledHeader.h"
#include "ResourceBindState.h"
#include "Utilities/Log.h"

ResourceBindState::ResourceBindState()
{
    GLint count;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &count);
    m_bindings[CG_TYPE_TEXTURE] = std::vector<GraphicsID>(count);

    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &count);
    m_bindings[CG_TYPE_CONSTANT_BUFFER] = std::vector<GraphicsID>(count);

    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &count);
    m_bindings[CG_TYPE_COMPUTE_BUFFER] = std::vector<GraphicsID>(count);
}

void ResourceBindState::BindTextures(ushort location, const GraphicsID* graphicsIds, ushort count)
{
    auto bindings = GetBindings(CG_TYPE_TEXTURE, location, count);

    for (ushort i = 0; i < count; ++i)
    {
        if (bindings[i] == graphicsIds[i])
        {
            continue;
        }

        bindings[i] = graphicsIds[i];
        glBindTextureUnit(location + i, graphicsIds[i]);
    }
}

void ResourceBindState::BindBuffers(ushort type, ushort location, const GraphicsID* graphicsIds, ushort count)
{
    auto bindings = GetBindings(type, location, count);
    auto nativeType = CGConvert::NativeEnum(type);

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

GraphicsID* ResourceBindState::GetBindings(ushort type, ushort location, ushort count)
{
    auto& bindings = m_bindings.at(type);
    auto* bbuff = bindings.data();
    PK_CORE_ASSERT((ushort)bindings.size() > (location + count), "MAXIMUM BUFFER BINDING INDEX EXCEEDED!");
    return bbuff + location;
}
