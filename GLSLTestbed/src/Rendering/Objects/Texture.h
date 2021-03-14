#pragma once
#include "Rendering/Objects/GraphicsObject.h"
#include <hlslmath.h>
#include <KTX/ktx.h>

struct TextureDescriptor
{
    uint3 resolution;
    GLenum colorFormat = GL_RGBA8;
    GLenum dimension = GL_TEXTURE_2D;
    GLenum wrapmodex = GL_REPEAT;
    GLenum wrapmodey = GL_REPEAT;
    GLenum filtermag = GL_LINEAR;
    GLenum filtermin = GL_LINEAR;
    uint miplevels = 0;
    uint msaaSamples = 0;
};

class Texture : public GraphicsObject
{
    public:
        Texture(const TextureDescriptor& descriptor);
        virtual ~Texture() = 0 {};

        TextureDescriptor GetDescriptor() const { return m_descriptor; }
        uint2 GetResolution2D() const { return m_descriptor.resolution.xy; }
        uint3 GetResolution3D() const { return m_descriptor.resolution; }
        uint GetWidth() const { return m_descriptor.resolution.x; }
        uint GetHeight() const { return m_descriptor.resolution.y; }
        uint GetDepth() const { return m_descriptor.resolution.z; }
        uint GetSize() const { return m_descriptor.resolution.x * m_descriptor.resolution.y * m_descriptor.resolution.z * GetTexelSize(m_descriptor.colorFormat); }
        uint8_t GetChannelCount() const { return GetChannelCount(m_channels); }
        GLenum GetDimension() const { return m_descriptor.dimension; }

        void SetWrapMode(GLenum x, GLenum y);
        void SetFilterMode(GLenum min, GLenum mag);

    protected:
        void SetDescriptor(const TextureDescriptor& descriptor);

        static void CreateTextureStorage(GraphicsID& graphicsId, const TextureDescriptor& descriptor);
        static GLenum GetFormatChannels(GLenum format);
        static GLenum GetFormatDataType(GLenum format);
        static uint8_t GetTexelSize(GLenum format);
        static uint8_t GetChannelCount(GLenum channels);
        static void GetDescirptorFromKTX(ktxTexture* tex, TextureDescriptor* desc, GLenum* channels);

        TextureDescriptor m_descriptor;
        GLenum m_channels;
};

