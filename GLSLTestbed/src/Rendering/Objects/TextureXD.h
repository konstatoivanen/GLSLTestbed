#pragma once
#include "Core/AssetDataBase.h"
#include "Rendering/Objects/Texture.h"

class TextureXD : public Texture, public Asset
{
	friend void AssetImporters::Import(const std::string& filepath, Ref<TextureXD>& shader);

	public:
		TextureXD();
		TextureXD(const TextureDescriptor& descriptor);
		~TextureXD();
	
		void SetData(void* data, uint32_t size, uint32_t miplevel);
		void SetMipLevel(const Ref<TextureXD>& texture, uint32_t mipLevel);
};

