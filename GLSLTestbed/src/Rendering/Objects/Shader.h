#pragma once
#include "Core/AssetDataBase.h"
#include "Rendering/Objects/GraphicsObject.h"
#include "Rendering/Structs/ShaderPropertyBlock.h"
#include <hlslmath.h>

struct ShaderPropertyInfo
{
	GLint location;
	GLenum type;
};

struct ShaderStateParameters
{
	bool ZWriteEnabled;
	bool ZTestEnabled;
	bool BlendEnabled;
	bool CullEnabled;
	GLenum ZTest;
	GLenum BlendSrc;
	GLenum BlendDst;
	GLenum CullMode;
	unsigned char ColorMask;
};

class ShaderVariantMap
{
	public:
		void Reset();
		void SetKeywords(const uint32_t* hashIds, size_t count);
		uint32_t GetActiveIndex();

		uint32_t variantcount = 0;
		uint32_t directivecount = 0;
		uint32_t directives[16];
		std::unordered_map<uint32_t, uint8_t> keywords;
};

class ShaderVariant : public GraphicsObject
{
	public:
		ShaderVariant(GraphicsID graphicsId, const std::map<uint32_t, ShaderPropertyInfo>& properties);
		~ShaderVariant();
		void SetPropertyBlock(const ShaderPropertyBlock& propertyBlock);
		void ListProperties();
	private:
		std::map<uint32_t, ShaderPropertyInfo> m_properties;
};

class Shader: public Asset
{
	friend void AssetImporters::Import(const std::string& filepath, Ref<Shader>& shader);

	public:
		~Shader();
		const ShaderStateParameters& GetStateParameters() const { return m_stateParameters; }
		const Ref<ShaderVariant>& GetActiveVariant();

		void SetPropertyBlock(const ShaderPropertyBlock& propertyBlock) { m_variants.at(m_activeIndex)->SetPropertyBlock(propertyBlock); }
		void ResetKeywords();
		void SetKeywords(const std::vector<uint32_t>& keywords);
		void ListProperties();

	private:
		uint32_t m_activeIndex = 0;
		std::vector<Ref<ShaderVariant>> m_variants;
		ShaderVariantMap m_variantMap = ShaderVariantMap();
		ShaderStateParameters m_stateParameters = ShaderStateParameters();
};
