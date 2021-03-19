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
	m_colorBuffers.clear();
	m_depthBuffer = nullptr;
}

void RenderTexture::Rebuild(const RenderTextureDescriptor& descriptor)
{
	if (m_graphicsId)
	{
		glDeleteFramebuffers(1, &m_graphicsId);
	}

	auto colorCount = m_bufferAttachmentCount = descriptor.colorFormats.size();

	PK_CORE_ASSERT(descriptor.depthFormat != GL_NONE || colorCount > 0, "RenderTextureDescriptor doesn't specify at least 1 valid format");
	PK_CORE_ASSERT(colorCount < MAX_RENDER_BUFFER_COUNT, "Maximum render buffer attachment counte exceeded!");

	SetDescriptor(ConvertDescriptor(descriptor, colorCount ? descriptor.colorFormats.at(0) : GL_NONE));
	
	m_compoundDescriptor = descriptor;
	m_depthBuffer = nullptr;
	m_colorBuffers.clear();
	
	glCreateFramebuffers(1, &m_graphicsId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_graphicsId);

	if (colorCount > 0)
	{
		m_colorBuffers.resize(colorCount);
		
		for (auto i = 0; i < colorCount; ++i)
		{
			RenderBuffer::Validate(m_colorBuffers[i], ConvertDescriptor(descriptor, descriptor.colorFormats.at(i)));
			Graphics::SetRenderBuffer(m_colorBuffers[i], GL_COLOR_ATTACHMENT0 + i);
			m_bufferAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}
	}

	if (descriptor.depthFormat != GL_NONE)
	{
		RenderBuffer::Validate(m_depthBuffer, ConvertDescriptor(descriptor, descriptor.depthFormat));
		Graphics::SetRenderBuffer(m_depthBuffer, GL_DEPTH_STENCIL_ATTACHMENT);
		m_bufferAttachments[colorCount] = GL_DEPTH_STENCIL_ATTACHMENT;
		++m_bufferAttachmentCount;
	}

	if (colorCount > 0)
	{
		glDrawBuffers((GLsizei)colorCount, m_bufferAttachments);
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

void RenderTexture::SetDrawTargets(std::initializer_list<GLenum> attachements)
{
	if (attachements.size() > 0)
	{
		glNamedFramebufferDrawBuffers(m_graphicsId, attachements.size(), attachements.begin());
	}
	else
	{
		glNamedFramebufferDrawBuffer(m_graphicsId, GL_NONE);
	}
}

void RenderTexture::ResetDrawTargets()
{
	if (m_colorBuffers.size() > 0)
	{
		glNamedFramebufferDrawBuffers(m_graphicsId, (GLsizei)m_colorBuffers.size(), m_bufferAttachments);
	}
	else
	{
		glNamedFramebufferDrawBuffer(m_graphicsId, GL_NONE);
	}
}

void RenderTexture::DiscardContents()
{
	glInvalidateNamedFramebufferData(m_graphicsId, (GLsizei)m_bufferAttachmentCount, m_bufferAttachments);
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
