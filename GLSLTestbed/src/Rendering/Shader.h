#pragma once
#include "PrecompiledHeader.h"
#include "Rendering/ShaderPropertyBlock.h"
#include "Rendering/GraphicsObject.h"
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

class Shader: public GraphicsObject
{
	friend class ShaderCompiler;
	friend class ShaderCollection;

	public:
		~Shader();

		uint32_t GetHashId() const { return m_hashId; }
		const std::string& GetName() const { return m_name; }
		const std::string& GetFilePath() const { return m_cachedFilepath; }

		bool HasProperty(uint32_t hashId) const { return m_variableMap.count(hashId) > 0; };
		const ShaderPropertyInfo& GetProperty(uint32_t hashId) const { return m_variableMap.at(hashId); }
		const ShaderStateParameters& GetStateParameters() const { return m_stateParameters; }

		void ListUniforms();
		void SetPropertyBlock(const ShaderPropertyBlock& propertyBlock);

	private:
		uint32_t m_hashId = 0;
		std::string m_name;
		std::string m_cachedFilepath;
		std::map<uint32_t, ShaderPropertyInfo> m_variableMap;
		ShaderStateParameters m_stateParameters = ShaderStateParameters();
};

class ShaderCompiler
{
	public:
		static std::string ReadFile(const std::string& filepath);
		static std::unordered_map<GLenum, std::string> Preprocess(std::string& source, ShaderStateParameters& stateParameters);
		static void Compile(Shader* shader, const std::unordered_map<GLenum, std::string>& shaderSources);
};

class ShaderCollection
{
	public:
		Ref<Shader> Import(const std::string& filename);
		void Reimport(Ref<Shader> shader);
		void ImportCollection(const std::string& filename);
		void Release();

		Ref<Shader> Find(const std::string& name);
		Ref<Shader> Find(const uint32_t hashId);
		bool FindActive(Ref<Shader>& shader);

		std::vector<uint32_t> GetAllShaderHashIds();

	private:
		Ref<Shader> GetShader(const uint32_t hashId);
		std::unordered_map<uint32_t, Ref<Shader>> m_shaderCollection;
		std::unordered_map<GraphicsID, uint32_t> m_shaderProgramMap;
};
