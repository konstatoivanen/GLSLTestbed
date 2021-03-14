#include "PrecompiledHeader.h"
#include "Rendering/Objects/RenderTexture.h"
#include "Rendering/Graphics.h"

static TextureDescriptor ConvertDescriptor(const RenderTextureDescriptor& descriptor, GLenum colorFromat)
{
	auto textureDescriptor = TextureDescriptor();
	textureDescriptor.resolution = descriptor.resolution;
	textureDescriptor.colorFormat = colorFromat;
	textureDescriptor.dimension = descriptor.dimension;
	textureDescriptor.wrapmodex = descriptor.wrapmodex;
	textureDescriptor.wrapmodey = descriptor.wrapmodey;
	textureDescriptor.filtermag = descriptor.filtermag;
	textureDescriptor.filtermin = descriptor.filtermin;
	textureDescriptor.miplevels = descriptor.miplevels;
	textureDescriptor.msaaSamples = descriptor.msaaSamples;
	return textureDescriptor;
}

RenderTexture::RenderTexture(const RenderTextureDescriptor& descriptor) : Texture(TextureDescriptor())
{
	Rebuild(descriptor);
}

RenderTexture::~RenderTexture()
{
	glDeleteFramebuffers(1, &m_graphicsId);
	colorBuffers.clear();
	depthBuffer = nullptr;
}

void RenderTexture::Rebuild(const RenderTextureDescriptor& descriptor)
{
	if (m_graphicsId)
	{
		glDeleteFramebuffers(1, &m_graphicsId);
	}

	auto colorCount = descriptor.colorFormats.size();

	PK_CORE_ASSERT(descriptor.depthFormat != GL_NONE || colorCount > 0, "RenderTextureDescriptor doesn't specify at least 1 valid format");

	SetDescriptor(ConvertDescriptor(descriptor, colorCount ? descriptor.colorFormats.at(0) : GL_NONE));
	m_compoundDescriptor = descriptor;

	depthBuffer = nullptr;
	colorBuffers.clear();
	
	glCreateFramebuffers(1, &m_graphicsId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_graphicsId);

	auto* buffers = colorCount > 0 ? PK_STACK_ALLOC(GLenum, colorCount) : nullptr;
	
	if (colorCount > 0)
	{
		colorBuffers.resize(colorCount);
		
		for (auto i = 0; i < colorCount; ++i)
		{
			RenderBuffer::Validate(colorBuffers[i], ConvertDescriptor(descriptor, descriptor.colorFormats.at(i)));
			Graphics::SetRenderBuffer(colorBuffers[i], GL_COLOR_ATTACHMENT0 + i);
			buffers[i] = GL_COLOR_ATTACHMENT0 + i;
		}
	}

	if (descriptor.depthFormat != GL_NONE)
	{
		RenderBuffer::Validate(depthBuffer, ConvertDescriptor(descriptor, descriptor.depthFormat));
		Graphics::SetRenderBuffer(depthBuffer, GL_DEPTH_STENCIL_ATTACHMENT);
	}

	if (colorCount > 0)
	{
		glDrawBuffers((GLsizei)colorCount, buffers);
	}
	else
	{
		glDrawBuffer(GL_NONE);
	}

	PK_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::ValidateResolution(const uint3& resolution)
{
	if (m_compoundDescriptor.resolution != resolution)
	{
		m_compoundDescriptor.resolution = resolution;
		Rebuild(m_compoundDescriptor);
	}
}

void RenderTexture::DiscardContents()
{
	auto colorCount = colorBuffers.size();
	auto depthCount = depthBuffer != nullptr ? 1 : 0;
	auto count = colorCount + depthCount;

	auto* buffers = count > 0 ? PK_STACK_ALLOC(GLenum, count) : nullptr;

	for (auto i = 0; i < colorCount; ++i)
	{
		buffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	if (depthCount > 0)
	{
		buffers[colorCount] = GL_DEPTH_STENCIL_ATTACHMENT;
	}

	glInvalidateFramebuffer(GL_FRAMEBUFFER, (GLsizei)count, buffers);

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
