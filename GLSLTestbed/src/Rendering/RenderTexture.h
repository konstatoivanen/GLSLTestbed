#pragma once
#include "Utilities/Ref.h"
#include "Rendering/Texture.h"

struct RenderTextureDescriptor : TextureDescriptor
{
	GLenum depthFormat = GL_DEPTH24_STENCIL8;
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
        Ref<RenderBuffer> colorBuffer;
        Ref<RenderBuffer> depthBuffer;
};