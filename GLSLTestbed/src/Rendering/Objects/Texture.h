#pragma once
#include "Rendering/Objects/GraphicsObject.h"
#include <glad/glad.h>
#include <KTX/ktx.h>

struct TextureDescriptor
{
    uint32_t width = 0, height = 0, depth = 0;
    GLenum colorFormat = GL_RGBA8;
    GLenum dimension = GL_TEXTURE_2D;
    GLenum wrapmodex = GL_REPEAT;
    GLenum wrapmodey = GL_REPEAT;
    GLenum filtermag = GL_LINEAR;
    GLenum filtermin = GL_LINEAR;
    uint32_t miplevels = 0;
    uint32_t msaaSamples = 0;
};

class Texture : public GraphicsObject
{
    public:
        Texture(const TextureDescriptor& descriptor);
        virtual ~Texture() = default;

        TextureDescriptor GetDescriptor() const { return m_descriptor; }
        uint32_t GetWidth() const { return m_descriptor.width; }
        uint32_t GetHeight() const { return m_descriptor.height; }
        uint32_t GetSize() const { return m_descriptor.width * m_descriptor.height * GetTexelSize(m_descriptor.colorFormat); }
        uint8_t GetChannelCount() const { return GetChannelCount(m_channels); }
        GLenum GetDimension() const { return m_descriptor.dimension; }

        void SetWrapMode(GLenum x, GLenum y);
        void SetFilterMode(GLenum min, GLenum mag);

    protected:
        void SetDescriptor(const TextureDescriptor& descriptor);

        static void CreateTextureStorage(GraphicsID& graphicsId, const TextureDescriptor& descriptor);
        static GLenum GetFormatChannels(GLenum format);
        static uint8_t GetTexelSize(GLenum format);
        static uint8_t GetChannelCount(GLenum channels);
        static void GetDescirptorFromKTX(ktxTexture* tex, TextureDescriptor* desc, GLenum* channels);

        TextureDescriptor m_descriptor;
        GLenum m_channels;
};

