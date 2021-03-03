#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/Texture.h"

Texture::Texture(const TextureDescriptor& descriptor) : m_descriptor(descriptor), m_channels(GetFormatChannels(descriptor.colorFormat))
{
}

void Texture::SetWrapMode(GLenum x, GLenum y)
{
    m_descriptor.wrapmodex = x;
    m_descriptor.wrapmodey = y;
    glTextureParameteri(m_graphicsId, GL_TEXTURE_WRAP_S, x);
    glTextureParameteri(m_graphicsId, GL_TEXTURE_WRAP_T, y);
}

void Texture::SetFilterMode(GLenum min, GLenum mag)
{
    m_descriptor.filtermin = min;
    m_descriptor.filtermag = mag;
    glTextureParameteri(m_graphicsId, GL_TEXTURE_MIN_FILTER, min);
    glTextureParameteri(m_graphicsId, GL_TEXTURE_MAG_FILTER, mag);
}

void Texture::SetDescriptor(const TextureDescriptor& descriptor)
{
    m_descriptor = descriptor;
    m_channels = GetFormatChannels(descriptor.colorFormat);
}

void Texture::CreateTextureStorage(GraphicsID& graphicsId, const TextureDescriptor& descriptor)
{
    // Add support for dimensions later
    glCreateTextures(GL_TEXTURE_2D, 1, &graphicsId);
    glTextureStorage2D(graphicsId, descriptor.miplevels > 0 ? descriptor.miplevels : 1, descriptor.colorFormat, descriptor.width, descriptor.height);
    glTextureParameteri(graphicsId, GL_TEXTURE_MIN_FILTER, descriptor.filtermin);
    glTextureParameteri(graphicsId, GL_TEXTURE_MAG_FILTER, descriptor.filtermag);
    glTextureParameteri(graphicsId, GL_TEXTURE_WRAP_S, descriptor.wrapmodex);
    glTextureParameteri(graphicsId, GL_TEXTURE_WRAP_T, descriptor.wrapmodey);
}

GLenum Texture::GetFormatChannels(GLenum format)
{
    switch (format)
    {
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
            return GL_RED;
        case GL_RG8:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG32F:
            return GL_RG;
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB565:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB16:
        case GL_RGB16F:
        case GL_RGB32F:
            return GL_RGB;
        case GL_RGBA4:
        case GL_RGBA8:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA32F:
            return GL_RGBA;
        case GL_DEPTH24_STENCIL8:
            return GL_DEPTH_STENCIL;
    }

    PK_CORE_ERROR("UNSUPPORTED TEXTURE FORMAT");
}

uint8_t Texture::GetTexelSize(GLenum format)
{
    switch (format)
    {
        case GL_R8: return 8;
        case GL_R16: return 16;
        case GL_R16F: return 16;
        case GL_R32F: return 32;

        case GL_RG8: return 16; // 2 * 8
        case GL_RG16: return 32; // 2 * 16
        case GL_RG16F: return 32; // 2 * 16
        case GL_RG32F: return 64; // 2 * 32

        case GL_RGB565: return 16; // 5 + 6 + 5
        case GL_RGB4: return 12; // 3 * 4
        case GL_RGB5: return 15; // 3 * 5
        case GL_RGB8: return 24; //3 * 8
        case GL_RGB10: return 30; // 3 * 10
        case GL_RGB16: return 48; // 3 * 16
        case GL_RGB16F: return 48; // 3 * 16
        case GL_RGB32F: return 96; // 3 * 32

        case GL_RGBA4: return 16; // 4 * 4
        case GL_RGBA8: return 32; // 4 * 8
        case GL_RGBA12: return 48; // 4 * 12
        case GL_RGBA16: return 64; // 4 * 16
        case GL_RGBA16F: return 64; // 4 * 16
        case GL_RGBA32F: return 128;  //4 * 32

        case GL_DEPTH24_STENCIL8: return 32;
    }

    PK_CORE_ERROR("UNSUPPORTED TEXTURE FORMAT");
}

uint8_t Texture::GetChannelCount(GLenum channels)
{
    switch (channels)
    {
        case GL_RED: return 1;
        case GL_RG: return 2;
        case GL_RGB: return 3;
        case GL_RGBA: return 4;
        case GL_DEPTH_STENCIL: return 1;
    }

    PK_CORE_ERROR("UNSUPPORTED CHANNEL FORMAT");
}

void Texture::GetDescirptorFromKTX(ktxTexture* tex, TextureDescriptor* desc, GLenum* channels)
{
    PK_CORE_ASSERT(tex->classId == ktxTexture1_c,"KTX2 is unsupported!")

    auto* tex1 = reinterpret_cast<ktxTexture1*>(tex);

    *channels = tex1->glFormat;

    desc->width = tex1->baseWidth;
    desc->height = tex1->baseHeight;
    desc->depth = tex1->baseDepth;
    desc->colorFormat = tex1->glInternalformat;
    desc->wrapmodex = GL_CLAMP_TO_EDGE;
    desc->wrapmodey = GL_CLAMP_TO_EDGE;
    desc->filtermin = GL_NEAREST;
    desc->filtermag = GL_LINEAR;
    desc->miplevels = tex1->numLevels;

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
