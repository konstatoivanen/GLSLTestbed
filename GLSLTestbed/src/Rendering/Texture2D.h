#pragma once
#include "PreCompiledHeader.h"
#include "Rendering/GraphicsObject.h"
#include "Rendering/Texture.h"
#include <GL\glew.h>

class Texture2D : public Texture
{
	public:
		Texture2D(const TextureDescriptor& descriptor);
		Texture2D(const TextureDescriptor& descriptor, const std::string& path);
		Texture2D(const std::string& path);
		~Texture2D();
	
		void SetData(void* data, uint32_t size);

	private:
		std::string m_Path;
};

