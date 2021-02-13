#include "PrecompiledHeader.h"
#include "Rendering/RenderTexture.h"
#include "Rendering/Graphics.h"

RenderTexture::RenderTexture(const RenderTextureDescriptor& descriptor) : Texture(descriptor)
{
	Rebuild(descriptor);
}

RenderTexture::~RenderTexture()
{
	glDeleteFramebuffers(1, &m_graphicsId);
	colorBuffer = nullptr;
	depthBuffer = nullptr;
}

void RenderTexture::Rebuild(const RenderTextureDescriptor& descriptor)
{
	if (m_graphicsId)
	{
		glDeleteFramebuffers(1, &m_graphicsId);
	}

	SetDescriptor(descriptor);

	glCreateFramebuffers(1, &m_graphicsId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_graphicsId);
	
	if (descriptor.colorFormat != GL_NONE)
	{
		RenderBuffer::Validate(colorBuffer, descriptor);
		Graphics::SetRenderBuffer(colorBuffer, GL_COLOR_ATTACHMENT0);
	}

	if (descriptor.depthFormat != GL_NONE)
	{
		auto depthDescriptor = TextureDescriptor(descriptor);
		depthDescriptor.colorFormat = descriptor.depthFormat;
		RenderBuffer::Validate(depthBuffer, depthDescriptor);
		Graphics::SetRenderBuffer(depthBuffer, GL_DEPTH_STENCIL_ATTACHMENT);
	}

	PK_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderBuffer::RenderBuffer(const TextureDescriptor& descriptor) : Texture(descriptor)
{
	Rebuild(descriptor);
}

RenderBuffer::~RenderBuffer()
{
	glDeleteTextures(1, &m_graphicsId);
}

void RenderBuffer::Validate(Ref<RenderBuffer>& buffer, const TextureDescriptor& descriptor)
{
	if (buffer == nullptr)
	{
		buffer = CreateRef<RenderBuffer>(descriptor);
		return;
	}

	buffer->Rebuild(descriptor);
}

void RenderBuffer::Rebuild(const TextureDescriptor& descriptor)
{
	if (m_graphicsId)
	{
		glDeleteTextures(1, &m_graphicsId);
	}

	SetDescriptor(descriptor);
	CreateTextureStorage(m_graphicsId, descriptor);
}