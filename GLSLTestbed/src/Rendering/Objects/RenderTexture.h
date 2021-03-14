#pragma once
#include "Utilities/Ref.h"
#include "Rendering/Objects/Texture.h"

struct RenderTextureDescriptor
{
    uint3 resolution = CG_UINT3_ZERO;
    std::vector<GLenum> colorFormats = { GL_RGBA8 };
    GLenum depthFormat = GL_DEPTH24_STENCIL8;
    GLenum dimension = GL_TEXTURE_2D;
    GLenum wrapmodex = GL_REPEAT;
    GLenum wrapmodey = GL_REPEAT;
    GLenum filtermag = GL_LINEAR;
    GLenum filtermin = GL_LINEAR;
    uint32_t miplevels = 0;
    uint32_t msaaSamples = 0;
};

class RenderBuffer : public Texture
{
	public:
		RenderBuffer(const TextureDescriptor& descriptor);
		~RenderBuffer();
		static void Validate(Ref<RenderBuffer>& buffer, const TextureDescriptor& descriptor);
		void Rebuild(const TextureDescriptor& descriptor);
};

class RenderTexture : public Texture
{
    public:
		RenderTexture(const RenderTextureDescriptor& descriptor);
		~RenderTexture();
		void Rebuild(const RenderTextureDescriptor& descriptor);
        void DiscardContents();

        RenderTextureDescriptor GetCompoundDescriptor() { return m_compoundDescriptor; }
        Weak<RenderBuffer> GetColorBuffer(int index) const { return colorBuffers.at(index); }
        Weak<RenderBuffer> GetDepthBuffer() const { return depthBuffer; }
        size_t GetColorBufferCount() const { return colorBuffers.size(); }

    private:
        RenderTextureDescriptor m_compoundDescriptor;
        std::vector<Ref<RenderBuffer>> colorBuffers;
        Ref<RenderBuffer> depthBuffer;
};