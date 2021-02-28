#pragma once
#include "Core/AssetDataBase.h"
#include "Rendering/Objects/Texture.h"

class Texture2D : public Texture, public Asset
{
	friend void AssetImporters::Import(const std::string& filepath, Ref<Texture2D>& shader);

	public:
		Texture2D();
		Texture2D(const TextureDescriptor& descriptor);
		~Texture2D();
	
		void SetData(void* data, uint32_t size);
};

