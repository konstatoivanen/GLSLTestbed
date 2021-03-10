#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/Texture2D.h"
#include <stb_image.h>
#include <KTX/ktx.h>

Texture2D::Texture2D() : Texture(TextureDescriptor())
{
}

Texture2D::Texture2D(const TextureDescriptor& descriptor) : Texture(descriptor)
{
	CreateTextureStorage(m_graphicsId, descriptor);
}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &m_graphicsId);
}

void Texture2D::SetData(void* data, uint32_t size, uint32_t miplevel)
{
	PK_CORE_ASSERT(size == GetSize(), "Texture data size miss match");
	glTextureSubImage2D(m_graphicsId, miplevel, 0, 0, m_descriptor.width >> miplevel, m_descriptor.height >> miplevel, m_channels, GL_UNSIGNED_BYTE, data);
}

void Texture2D::SetMipLevel(const Ref<Texture2D>& texture, uint32_t miplevel)
{
	auto srcDescriptor = texture->GetDescriptor();
	auto src = texture->GetGraphicsID();
	auto dst = m_graphicsId;
	glCopyImageSubData(src, srcDescriptor.dimension, 0, 0, 0, 0, dst, m_descriptor.dimension, miplevel, 0, 0, 0, srcDescriptor.width, srcDescriptor.height, srcDescriptor.depth);
}

template<>
void AssetImporters::Import(const std::string& filepath, Ref<Texture2D>& texture)
{
	if (texture->m_graphicsId != 0)
	{
		glDeleteTextures(1, &texture->m_graphicsId);
	}

	if (filepath.find(".ktx") != std::string::npos)
	{
		ktxTexture* kTexture;
		KTX_error_code result;
		GLenum target, glerror;

		result = ktxTexture_CreateFromNamedFile(filepath.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);

		PK_CORE_ASSERT(result == KTX_SUCCESS, "Failed to load ktx!");

		Texture::GetDescirptorFromKTX(kTexture, &texture->m_descriptor, &texture->m_channels);

		glGenTextures(1, &texture->m_graphicsId);

		result = ktxTexture_GLUpload(kTexture, &texture->m_graphicsId, &target, &glerror);
		
		PK_CORE_ASSERT(result == KTX_SUCCESS, "Failed to upload ktx!");

		glTextureParameteri(texture->m_graphicsId, GL_TEXTURE_MIN_FILTER, texture->m_descriptor.filtermin);
		glTextureParameteri(texture->m_graphicsId, GL_TEXTURE_MAG_FILTER, texture->m_descriptor.filtermag);
		glTextureParameteri(texture->m_graphicsId, GL_TEXTURE_WRAP_S, texture->m_descriptor.wrapmodex);
		glTextureParameteri(texture->m_graphicsId, GL_TEXTURE_WRAP_T, texture->m_descriptor.wrapmodey);

		ktxTexture_Destroy(kTexture);
		return;
	}

	auto desiredChannelCount = Texture::GetChannelCount(texture->m_channels);

	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);

	stbi_uc* data = nullptr;
	{
		data = stbi_load(filepath.c_str(), &width, &height, &channels, desiredChannelCount);
	}

	PK_CORE_ASSERT(data, "Failed to load image!");
	PK_CORE_ASSERT(channels == desiredChannelCount, "Failed to load image with the desired channel count!");

	texture->m_descriptor.width = (uint32_t)width;
	texture->m_descriptor.height = (uint32_t)width;

	Texture::CreateTextureStorage(texture->m_graphicsId, texture->m_descriptor);
	texture->SetData(data, texture->GetSize(), 0);

	stbi_image_free(data);
}