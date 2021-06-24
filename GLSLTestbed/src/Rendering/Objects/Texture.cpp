#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/Texture.h"

namespace PK::Rendering::Objects
{
    using namespace Utilities;

    Texture::Texture(const TextureDescriptor& descriptor) : m_descriptor(descriptor), m_channels(GetFormatChannels(descriptor.colorFormat))
    {
    }
    
    GLuint64 Texture::GetBindlessHandleResident() const
    {
        auto handle = glGetTextureHandleARB(m_graphicsId);

        if (!glIsTextureHandleResidentARB(handle))
        {
            glMakeTextureHandleResidentARB(handle);
        }

        return handle;
    }

    GLuint64 Texture::GetImageHandleResident(GLenum format, GLenum access, int level, int layer, bool layered) const
    {
        auto handle = glGetImageHandleARB(m_graphicsId, level, layered, layer, format);

        if (!glIsImageHandleResidentARB(handle))
        {
            glMakeImageHandleResidentARB(handle, access);
        }

        return handle;
    }

    void Texture::Clear(uint level, const void* clearValue) const
    {
        glClearTexImage(m_graphicsId, level, m_channels, GetFormatDataType(m_descriptor.colorFormat), clearValue);
    }

    void Texture::SetWrapMode(GLenum x, GLenum y, GLenum z)
    {
        m_descriptor.wrapmodex = x;
        m_descriptor.wrapmodey = y;
        m_descriptor.wrapmodey = z;
        glTextureParameteri(m_graphicsId, GL_TEXTURE_WRAP_S, x);
        glTextureParameteri(m_graphicsId, GL_TEXTURE_WRAP_T, y);
        glTextureParameteri(m_graphicsId, GL_TEXTURE_WRAP_R, z);
    }
    
    void Texture::SetFilterMode(GLenum min, GLenum mag)
    {
        m_descriptor.filtermin = min;
        m_descriptor.filtermag = mag;
        glTextureParameteri(m_graphicsId, GL_TEXTURE_MIN_FILTER, min);
        glTextureParameteri(m_graphicsId, GL_TEXTURE_MAG_FILTER, mag);
    }

    void Texture::SetBorderColor(const color& color)
    {
        m_descriptor.bordercolor = color;
        glTexParameterfv(m_graphicsId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
    }
    
    void Texture::SetDescriptor(const TextureDescriptor& descriptor)
    {
        m_descriptor = descriptor;
        m_channels = GetFormatChannels(descriptor.colorFormat);
    }
    
    void Texture::CreateTextureStorage(GraphicsID& graphicsId, const TextureDescriptor& descriptor)
    {
        glCreateTextures(descriptor.dimension, 1, &graphicsId);

        switch (descriptor.dimension)
        {
            case GL_TEXTURE_1D: 
                glTextureStorage1D(graphicsId, descriptor.miplevels > 0 ? descriptor.miplevels : 1, descriptor.colorFormat, descriptor.resolution.x); 
                break;
            case GL_TEXTURE_1D_ARRAY: 
            case GL_TEXTURE_RECTANGLE: 
            case GL_TEXTURE_CUBE_MAP: 
            case GL_TEXTURE_2D: 
                glTextureStorage2D(graphicsId, descriptor.miplevels > 0 ? descriptor.miplevels : 1, descriptor.colorFormat, descriptor.resolution.x, descriptor.resolution.y); 
                break;
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                glTextureStorage3D(graphicsId, descriptor.miplevels > 0 ? descriptor.miplevels : 1, descriptor.colorFormat, descriptor.resolution.x, descriptor.resolution.y, descriptor.resolution.z * 6);
                break;
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_3D:
                glTextureStorage3D(graphicsId, descriptor.miplevels > 0 ? descriptor.miplevels : 1, descriptor.colorFormat, descriptor.resolution.x, descriptor.resolution.y, descriptor.resolution.z);
                break;
            case GL_TEXTURE_2D_MULTISAMPLE:
                glTexStorage2DMultisample(graphicsId, descriptor.msaaSamples, descriptor.colorFormat, descriptor.resolution.x, descriptor.resolution.y, GL_FALSE);
                return;
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                glTexStorage3DMultisample(graphicsId, descriptor.msaaSamples, descriptor.colorFormat, descriptor.resolution.x, descriptor.resolution.y, descriptor.resolution.z, GL_FALSE);
                return;
        }

        glTextureParameteri(graphicsId, GL_TEXTURE_MIN_FILTER, descriptor.filtermin);
        glTextureParameteri(graphicsId, GL_TEXTURE_MAG_FILTER, descriptor.filtermag);
        glTextureParameteri(graphicsId, GL_TEXTURE_WRAP_S, descriptor.wrapmodex);
        glTextureParameteri(graphicsId, GL_TEXTURE_WRAP_T, descriptor.wrapmodey);
        glTextureParameteri(graphicsId, GL_TEXTURE_WRAP_R, descriptor.wrapmodez);
        glTextureParameterfv(graphicsId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(descriptor.bordercolor));
    }
    
    GLenum Texture::GetFormatChannels(GLenum format)
    {
        switch (format)
        {
            case GL_R8I: 
            case GL_R8UI: 
            case GL_R16I:
            case GL_R16UI:
            case GL_R32I: 
            case GL_R32UI: 
                return GL_RED_INTEGER;
            case GL_R8:
            case GL_R16:
            case GL_R16F:
            case GL_R32F: 
                return GL_RED;
            case GL_RG8I:
            case GL_RG8UI:
            case GL_RG16I:
            case GL_RG16UI:
            case GL_RG32I:
            case GL_RG32UI:
                return GL_RG_INTEGER;
            case GL_RG8:
            case GL_RG16:
            case GL_RG16F:
            case GL_RG32F:
                return GL_RG;
            case GL_RGB8I:
            case GL_RGB8UI:
            case GL_RGB16I:
            case GL_RGB16UI:
            case GL_RGB32UI:
            case GL_RGB32I:
                return GL_RGB_INTEGER;
            case GL_RGB4:
            case GL_RGB5:
            case GL_RGB565:
            case GL_RGB8:
            case GL_RGB10:
            case GL_RGB16:
            case GL_RGB16F:
            case GL_RGB32F:
                return GL_RGB;
            case GL_RGBA8I:
            case GL_RGBA8UI:
            case GL_RGBA16I:
            case GL_RGBA16UI:
            case GL_RGBA32I:
            case GL_RGBA32UI:
                return GL_RGBA_INTEGER;
            case GL_RGBA4:
            case GL_RGBA8:
            case GL_RGBA12:
            case GL_RGBA16:
            case GL_RGBA16F:
            case GL_RGBA32F:
            case GL_RGB10_A2:
                return GL_RGBA;
            case GL_DEPTH24_STENCIL8:
                return GL_DEPTH_STENCIL;
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
            case GL_DEPTH_COMPONENT32F:
                return GL_DEPTH_COMPONENT;
        }
    
        PK_CORE_ERROR("UNSUPPORTED TEXTURE FORMAT");
    }
    
    GLenum Texture::GetFormatDataType(GLenum format)
    {
        switch (format)
        {
            case GL_R8:
            case GL_R8UI:
            case GL_RG8:
            case GL_RG8UI:
            case GL_RGB8:
            case GL_RGB8UI:
            case GL_RGBA8:
            case GL_RGBA8UI:
                return GL_UNSIGNED_BYTE;
            case GL_R16:
            case GL_R16UI:
            case GL_RG16:
            case GL_RG16UI:
            case GL_RGB16:
            case GL_RGB16UI:
            case GL_RGBA16:
            case GL_RGBA16UI:
                return GL_UNSIGNED_SHORT;
            case GL_R16F:
            case GL_RG16F:
            case GL_RGB16F:
            case GL_RGBA16F:
                return GL_FLOAT;
            case GL_R32F:
            case GL_RG32F:
            case GL_RGB32F:
            case GL_RGBA32F:
                return GL_FLOAT;
            case GL_RGB4:
            case GL_RGBA4:
                return GL_UNSIGNED_SHORT_4_4_4_4;
            case GL_RGB5:
                return GL_UNSIGNED_SHORT_5_5_5_1;
            case GL_RGB565:
                return GL_UNSIGNED_SHORT_5_6_5;
            case GL_RGB10:
            case GL_RGB10_A2:
            case GL_RGB10_A2UI:
                return GL_UNSIGNED_INT_10_10_10_2;
            case GL_R32UI:
            case GL_RG32UI:
            case GL_RGB32UI:
            case GL_RGBA32UI:
            case GL_RGBA12:
                return GL_UNSIGNED_INT;
            case GL_DEPTH24_STENCIL8:
                return GL_UNSIGNED_INT_24_8;
            case GL_DEPTH_COMPONENT16:
                return GL_UNSIGNED_SHORT;
            case GL_DEPTH_COMPONENT24:
                return GL_UNSIGNED_INT_24_8;
            case GL_DEPTH_COMPONENT32:
                return GL_UNSIGNED_INT;
            case GL_DEPTH_COMPONENT32F:
                return GL_FLOAT;
        }
    
        PK_CORE_ERROR("UNSUPPORTED TEXTURE FORMAT");
    }

    GLenum Texture::GetWrapmodeFromString(const char* string)
    {
        if (strstr(string, "_clamp")) return GL_CLAMP_TO_EDGE;
        if (strstr(string, "_repeat")) return GL_REPEAT;
        if (strstr(string, "_mirror")) return GL_MIRRORED_REPEAT;
        return GL_CLAMP_TO_EDGE;
    }
    
    uint8_t Texture::GetTexelSize(GLenum format)
    {
        switch (format)
        {
            case GL_R8: return 8;
            case GL_R8I: return 8;
            case GL_R8UI: return 8;
            case GL_R16: return 16;
            case GL_R16I: return 16;
            case GL_R16UI: return 16;
            case GL_R16F: return 16;
            case GL_R32I: return 32;
            case GL_R32UI: return 32;
            case GL_R32F: return 32;
    
            case GL_RG8: return 16; // 2 * 8
            case GL_RG8I: return 16; // 2 * 8
            case GL_RG8UI: return 16; // 2 * 8
            case GL_RG16: return 32; // 2 * 16
            case GL_RG16I: return 32; // 2 * 16
            case GL_RG16UI: return 32; // 2 * 16
            case GL_RG16F: return 32; // 2 * 16
            case GL_RG32I: return 64; // 2 * 32
            case GL_RG32UI: return 64; // 2 * 32
            case GL_RG32F: return 64; // 2 * 32
    
            case GL_RGB565: return 16; // 5 + 6 + 5
            case GL_RGB4: return 12; // 3 * 4
            case GL_RGB5: return 15; // 3 * 5
            case GL_RGB8: return 24; //3 * 8
            case GL_RGB8I: return 24; //3 * 8
            case GL_RGB8UI: return 24; //3 * 8
            case GL_RGB10: return 30; // 3 * 10
            case GL_RGB16: return 48; // 3 * 16
            case GL_RGB16I: return 48; // 3 * 16
            case GL_RGB16UI: return 48; // 3 * 16
            case GL_RGB16F: return 48; // 3 * 16
            case GL_RGB32F: return 96; // 3 * 32
            case GL_RGB32I: return 96; // 3 * 32
            case GL_RGB32UI: return 96; // 3 * 32
    
            case GL_RGBA4: return 16; // 4 * 4
            case GL_RGBA8: return 32; // 4 * 8
            case GL_RGBA8I: return 32; // 4 * 8
            case GL_RGBA8UI: return 32; // 4 * 8
            case GL_RGB10_A2: return 32;  
            case GL_RGB10_A2UI: return 32;  
            case GL_RGBA12: return 48; // 4 * 12
            case GL_RGBA16: return 64; // 4 * 16
            case GL_RGBA16I: return 64; // 4 * 16
            case GL_RGBA16UI: return 64; // 4 * 16
            case GL_RGBA16F: return 64; // 4 * 16
            case GL_RGBA32F: return 128;  //4 * 32
            case GL_RGBA32I: return 128;  //4 * 32
            case GL_RGBA32UI: return 128;  //4 * 32
    
            case GL_DEPTH24_STENCIL8: return 32;
            case GL_DEPTH_COMPONENT16: return 16;
            case GL_DEPTH_COMPONENT24: return 32;
            case GL_DEPTH_COMPONENT32: return 32;
            case GL_DEPTH_COMPONENT32F: return 32;
        }
    
        PK_CORE_ERROR("UNSUPPORTED TEXTURE FORMAT");
    }
    
    uint8_t Texture::GetChannelCount(GLenum channels)
    {
        switch (channels)
        {
            case GL_RED: return 1;
            case GL_RED_INTEGER: return 1;
            case GL_RG: return 2;
            case GL_RG_INTEGER: return 2;
            case GL_RGB: return 3;
            case GL_RGB_INTEGER: return 3;
            case GL_RGBA: return 4;
            case GL_RGBA_INTEGER: return 4;
            case GL_DEPTH_STENCIL: return 1;
            case GL_DEPTH_COMPONENT: return 1;
        }
    
        PK_CORE_ERROR("UNSUPPORTED CHANNEL FORMAT");
    }
    
    void Texture::GetDescirptorFromKTX(ktxTexture* tex, TextureDescriptor* desc, GLenum* channels)
    {
        PK_CORE_ASSERT(tex->classId == ktxTexture1_c,"KTX2 is unsupported!")
    
        auto* tex1 = reinterpret_cast<ktxTexture1*>(tex);
    
        *channels = tex1->glFormat;
    
        desc->resolution = { tex1->baseWidth, tex1->baseHeight, tex1->baseDepth };
        desc->colorFormat = tex1->glInternalformat;
        desc->wrapmodex = GL_CLAMP_TO_EDGE;
        desc->wrapmodey = GL_CLAMP_TO_EDGE;
        desc->wrapmodez = GL_CLAMP_TO_EDGE;
        desc->filtermin = GL_NEAREST;
        desc->filtermag = GL_LINEAR;
        desc->miplevels = tex1->numLevels;
    
        if (tex1->numLevels > 1)
        {
            desc->filtermin = GL_LINEAR_MIPMAP_LINEAR;
        }
    
        switch (tex1->numDimensions)
        {
            case 1: desc->dimension = tex1->isArray ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D; break;
            case 2: desc->dimension = tex1->isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D; break;
            case 3: desc->dimension = GL_TEXTURE_3D; break;
            default: PK_CORE_ERROR("Invalid Texture Dimension");
        }
    
        if (tex1->isCubemap)
        {
            desc->dimension = tex1->isArray ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
        }
    }
}