#include "PrecompiledHeader.h"
#include "Utilities/Log.h"
#include "Rendering/Texture2D.h"
#include <stb_image.h>

Texture2D::Texture2D(const TextureDescriptor& descriptor) : Texture(descriptor)
{
	CreateTextureStorage(m_graphicsId, descriptor);
}

Texture2D::Texture2D(const TextureDescriptor& descriptor, const std::string& path) : Texture(descriptor), m_Path(path)
{
	auto desiredChannelCount = GetChannelCount(m_channels);

	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);

	stbi_uc* data = nullptr;
	{
		data = stbi_load(path.c_str(), &width, &height, &channels, desiredChannelCount);
	}

	PK_CORE_ASSERT(data, "Failed to load image!");
	PK_CORE_ASSERT(channels == desiredChannelCount, "Failed to load image with the desired channel count!");

	m_descriptor.width = (uint32_t)width;
	m_descriptor.height = (uint32_t)width;

	CreateTextureStorage(m_graphicsId, m_descriptor);
	glTextureSubImage2D(m_graphicsId, 0, 0, 0, m_descriptor.width, m_descriptor.height, m_channels, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
}

Texture2D::Texture2D(const std::string& path) : Texture2D(TextureDescriptor(), path)
{
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