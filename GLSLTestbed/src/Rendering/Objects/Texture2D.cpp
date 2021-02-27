#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Objects/Texture2D.h"
#include <stb_image.h>

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

void Texture2D::SetData(void* data, uint32_t size)
{
	PK_CORE_ASSERT(size == GetSize(), "Texture data size miss match");
	glTextureSubImage2D(m_graphicsId, 0, 0, 0, m_descriptor.width, m_descriptor.height, m_channels, GL_UNSIGNED_BYTE, data);
}

template<typename T>
void AssetImporters::Import(const std::string& filepath, Ref<T>& texture)
{
	if (texture->m_graphicsId != 0)
	{
		glDeleteTextures(1, &texture->m_graphicsId);
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
	texture->SetData(data, texture->GetSize());

	stbi_image_free(data);
}

template void AssetImporters::Import<Texture2D>(const std::string& filepath, Ref<Texture2D>& shader);