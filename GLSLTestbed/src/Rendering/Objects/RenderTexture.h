#pragma once
#include "Utilities/Ref.h"
#include "Rendering/Objects/Texture.h"

namespace PK::Rendering::Objects
{
    using namespace PK::Utilities;
    using namespace PK::Math;

    struct RenderTextureDescriptor
    {
        uint3 resolution = CG_UINT3_ZERO;
        std::vector<GLenum> colorFormats = { GL_RGBA8 };
        GLenum depthFormat = GL_DEPTH24_STENCIL8;
        GLenum dimension = GL_TEXTURE_2D;
        GLenum wrapmodex = GL_REPEAT;
        GLenum wrapmodey = GL_REPEAT;
        GLenum wrapmodez = GL_REPEAT;
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
            enum : uint { MAX_RENDER_BUFFER_COUNT = 32 };
    
    		RenderTexture(const RenderTextureDescriptor& descriptor);
    		~RenderTexture();
    		void Rebuild(const RenderTextureDescriptor& descriptor);
            bool ValidateResolution(const uint3& resolution);
            void SetDrawTargets(std::initializer_list<GLenum> attachements);
            void ResetDrawTargets();
            void DiscardContents();
    
            RenderTextureDescriptor GetCompoundDescriptor() const { return m_compoundDescriptor; }
            const RenderBuffer* GetColorBuffer(int index) const { return m_colorBuffers.at(index).get(); }
            const RenderBuffer* GetDepthBuffer() const { return m_depthBuffer.get(); }
            size_t GetColorBufferCount() const { return m_colorBuffers.size(); }
    
        private:
            RenderTextureDescriptor m_compoundDescriptor;
            std::vector<Ref<RenderBuffer>> m_colorBuffers;
            Ref<RenderBuffer> m_depthBuffer;
            GLenum m_bufferAttachments[MAX_RENDER_BUFFER_COUNT];
            size_t m_bufferAttachmentCount;
    };
}