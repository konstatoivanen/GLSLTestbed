#include "PrecompiledHeader.h"
#include "Rendering/Graphics.h"
#include "Rendering/Objects/Shader.h"
#include "Utilities/StringHashID.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/Log.h"
#include <ext.hpp>

void ShaderVariantMap::Reset()
{
	for (auto& i : directives)
	{
		i = i & 0xFFFFFF;
	}
}

void ShaderVariantMap::SetKeywords(const uint32_t* hashIds, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		auto hashId = hashIds[i];

		if (keywords.count(hashId) > 0)
		{
			auto kv = keywords.at(hashId);
			directives[kv >> 4] = ((kv & 0xF) << 24) | (directives[kv >> 4] & 0xFFFFFF);
		}
	}
}

uint32_t ShaderVariantMap::GetActiveIndex()
{
	uint32_t idx = 0;

	for (uint32_t i = 0; i < directivecount; ++i)
	{
		idx += (directives[i] & 0xFFFFFF) * (directives[i] >> 24);
	}

	return idx;
}


ShaderVariant::ShaderVariant(GraphicsID graphicsId, const std::map<uint32_t, ShaderPropertyInfo>& properties)
{
	m_graphicsId = graphicsId;
	m_properties = properties;
}

ShaderVariant::~ShaderVariant() { glDeleteProgram(m_graphicsId); }

void ShaderVariant::ListProperties()
{
	for (auto& i : m_properties)
	{
		PK_CORE_LOG("%s : %i", StringHashID::IDToString(i.first).c_str(), i.second.location);
	}
}

void ShaderVariant::SetPropertyBlock(const ShaderPropertyBlock& propertyBlock)
{
	for (auto& i : propertyBlock)
	{
		auto hashId = i.first;

		if (m_properties.count(hashId) < 1)
		{
			continue;
		}

		const auto& prop = m_properties.at(hashId);
		auto& info = i.second;
		uint count = info.size / CGType::Size(info.type);
		auto components = CGType::Components(info.type);

		switch (info.type)
		{
			case CG_TYPE_FLOAT: glUniform1fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_FLOAT2: glUniform2fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_FLOAT3: glUniform3fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_FLOAT4: glUniform4fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_FLOAT2X2: glUniformMatrix2fv(prop.location, count, GL_FALSE, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_FLOAT3X3: glUniformMatrix3fv(prop.location, count, GL_FALSE, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_FLOAT4X4: glUniformMatrix4fv(prop.location, count, GL_FALSE, propertyBlock.GetElementPtr<float>(info)); break;
			case CG_TYPE_INT:  glUniform1iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
			case CG_TYPE_INT2: glUniform2iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
			case CG_TYPE_INT3: glUniform3iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
			case CG_TYPE_INT4: glUniform4iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
			case CG_TYPE_TEXTURE: 
			{
				auto textureIds = propertyBlock.GetElementPtr<GraphicsID>(info);
				for (uint i = 0; i < count; ++i)
				{
					glBindTextureUnit(prop.location + i, textureIds[i]);
				}
				break;
			}
			case CG_TYPE_CONSTANT_BUFFER:
			{
				auto bufferIds = propertyBlock.GetElementPtr<GraphicsID>(info);
				for (uint i = 0; i < count; ++i)
				{
					glBindBufferBase(GL_UNIFORM_BUFFER, prop.location + i, bufferIds[i]);
				}
				break;
			}
			default: PK_CORE_ERROR("Invalid Shader Property Type");
		}
	}
}


Shader::~Shader() { m_variants.clear(); }

const Ref<ShaderVariant>& Shader::GetActiveVariant()
{
	m_activeIndex = m_variantMap.GetActiveIndex();
	return m_variants.at(m_activeIndex);
}

void Shader::ListProperties()
{
	PK_CORE_LOG_HEADER("Listing uniforms for shader: %s", GetFileName());
	m_variants.at(m_activeIndex)->ListProperties();
}

void Shader::ResetKeywords() { m_variantMap.Reset(); }

void Shader::SetKeywords(const std::vector<uint32_t>& keywords)
{
	if (keywords.size() > 0)
	{
		m_variantMap.SetKeywords(&keywords.at(0), keywords.size());
	}
}


namespace ShaderCompiler
{
	static GLint GetUniformResourceSlot(GLint& currentSlot, GLenum variableType)
	{
		// Add support for more types later
		switch (variableType)
		{
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
				return currentSlot++;
		}
	
		return -1;
	}
	
	static GLenum GetShaderTypeFromString(const std::string& type)
	{
		if (type == "VERTEX")
		{
			return GL_VERTEX_SHADER;
		}
	
		if (type == "FRAGMENT")
		{
			return GL_FRAGMENT_SHADER;
		}
	
		if (type == "GEOMETRY")
		{
			return GL_GEOMETRY_SHADER;
		}
	
		if (type == "TESSELATION_CONTROL")
		{
			return GL_TESS_CONTROL_SHADER;
		}
	
		if (type == "TESSELATION_EVALUATE")
		{
			return GL_TESS_EVALUATION_SHADER;
		}
	
		if (type == "GL_COMPUTE_SHADER")
		{
			return GL_COMPUTE_SHADER;
		}
	
		PK_CORE_ASSERT(false, "Unknown shader type!");
		return 0;
	}
	
	static void GetBlendModeFromString(const std::string& blendMode, GLenum& blendmode)
	{
		if (blendMode == "Zero")
		{
			blendmode = GL_ZERO;
		}
		else if (blendMode == "One")
		{
			blendmode = GL_ONE;
		}
		else if (blendMode == "SrcColor")
		{
			blendmode = GL_SRC_COLOR;
		}
		else if (blendMode == "OneMinusSrcColor")
		{
			blendmode = GL_ONE_MINUS_SRC_COLOR;
		}
		else if (blendMode == "DstColor")
		{
			blendmode = GL_DST_COLOR;
		}
		else if (blendMode == "OneMinusDstColor")
		{
			blendmode = GL_ONE_MINUS_DST_COLOR;
		}
		else if (blendMode == "SrcAlpha")
		{
			blendmode = GL_SRC_ALPHA;
		}
		else if (blendMode == "OneMinusSrcAlpha")
		{
			blendmode = GL_ONE_MINUS_SRC_ALPHA;
		}
		else if (blendMode == "DstAlpha")
		{
			blendmode = GL_DST_ALPHA;
		}
		else if (blendMode == "OneMinusDstAlpha")
		{
			blendmode = GL_ONE_MINUS_DST_ALPHA;
		}
		else if (blendMode == "ConstColor")
		{
			blendmode = GL_CONSTANT_COLOR;
		}
		else if (blendMode == "OneMinusConstColor")
		{
			blendmode = GL_ONE_MINUS_CONSTANT_COLOR;
		}
		else if (blendMode == "ConstAlpha")
		{
			blendmode = GL_CONSTANT_ALPHA;
		}
		else if (blendMode == "OneMinusConstAlpha")
		{
			blendmode = GL_ONE_MINUS_CONSTANT_ALPHA;
		}
		else
		{
			PK_CORE_ASSERT(false, "Unknown blend mode!");
		}
	}
	
	static void GetColorMaskFromString(const std::string& colorMask, unsigned char& mask)
	{
		if (colorMask.empty())
		{
			mask = 255;
			return;
		}

		mask = 0;
	
		if (colorMask.find('R') != std::string::npos)
		{
			mask |= 1 << 0;
		}
	
		if (colorMask.find('G') != std::string::npos)
		{
			mask |= 1 << 1;
		}
	
		if (colorMask.find('B') != std::string::npos)
		{
			mask |= 1 << 2;
		}
	
		if (colorMask.find('A') != std::string::npos)
		{
			mask |= 1 << 3;
		}
	}
	
	static bool GetZTestFromString(const std::string& ztest, GLenum& mode)
	{
		if (ztest == "Off")
		{
			mode = GL_ALWAYS;
			return false;
		}
		else if (ztest == "Always") 
		{
			mode = GL_ALWAYS;
			return true;
		}
		else if (ztest == "Never")
		{
			mode = GL_NEVER;
			return true;
		}
		else if (ztest == "Less") 
		{
			mode = GL_LESS;
			return true;
		}
		else if (ztest == "LEqual") 
		{
			mode = GL_LEQUAL;
			return true;
		}
		else if (ztest == "Greater") 
		{
			mode = GL_GREATER;
			return true;
		}
		else if (ztest == "NotEqual") 
		{
			mode = GL_NOTEQUAL;
			return true;
		}
		else if (ztest == "GEqual") 
		{
			mode = GL_GEQUAL;
			return true;
		}
		
		PK_CORE_ASSERT(false, "Invalid Argument type for ZTest value");
		return false;
	}
	
	static void GetCullModeFromString(const std::string& cull, GLenum& mode, bool& enabled)
	{
		if (cull.empty())
		{
			enabled = false;
			return;
		}

		if (cull == "Off") 
		{
			mode = GL_BACK;
			enabled = false;
			return;
		}
		else if (cull == "Front") 
		{
			mode = GL_FRONT;
			enabled = true;
			return;
		}
		else if (cull == "Back") 
		{
			mode = GL_BACK;
			enabled = true;
			return;
		}
	
		PK_CORE_ASSERT(false, "Invalid Argument type for Cull value");
	}
	
	static void GetSharedInclude(const std::string& source, std::string& sharedInclude)
	{
		auto typeToken = "#pragma PROGRAM_";
		auto typeTokenLength = strlen(typeToken);
		auto pos = source.find(typeToken, 0); //Start of shader type declaration line
	
		// Treat code in the beginning of the source as shared include
		if (pos != std::string::npos && pos != 0)
		{
			sharedInclude = source.substr(0, pos);
		}
	}

	static void GetVariantDefines(const std::vector<std::vector<std::string>>& keywords, uint32_t index, std::string& defines)
	{
		defines.clear();

		for (auto i = 0; i < keywords.size(); ++i)
		{
			auto& declares = keywords.at(i);
			auto& keyword = declares.at(index % declares.size());

			if (keyword != "_")
			{
				defines.append("#define ");
				defines.append(keyword);
				defines.append("\n");
			}

			index /= (uint32_t)declares.size();
		}
	}

	static void LogVariantDefines(const std::vector<std::vector<std::string>>& keywords, uint32_t index)
	{
		std::string defines = "";

		for (auto i = 0; i < keywords.size(); ++i)
		{
			auto& declares = keywords.at(i);
			auto& keyword = declares.at(index % declares.size());

			if (keyword != "_")
			{
				defines.append(keyword);
				defines.append(" ");
			}

			index /= (uint32_t)declares.size();
		}

		PK_CORE_LOG(defines.c_str());
	}
	
	static void ExtractMulticompiles(std::string& source, std::vector<std::vector<std::string>>& keywords, ShaderVariantMap& multicompilemap)
	{
		std::string output;
		size_t pos = 0;

		while (true)
		{
			pos = StringUtilities::ExtractToken(pos, "#multi_compile ", source, output, false);

			if (pos == std::string::npos)
			{
				break;
			}

			keywords.push_back(StringUtilities::Split(output, " "));
		}

		multicompilemap.variantcount = 1;
		multicompilemap.directivecount = (uint32_t)keywords.size();
		multicompilemap.keywords.clear();

		for (auto i = 0; i < keywords.size(); ++i)
		{
			auto& directive = keywords.at(i);

			for (auto j = 0; j < directive.size(); ++j)
			{
				auto& keyword = directive.at(j);

				if (keyword != "_")
				{
					multicompilemap.keywords[StringHashID::StringToID(keyword)] = (i << 4) | (j & 0xF);
				}
			}

			multicompilemap.directives[i] = multicompilemap.variantcount & 0xFFFFFF;
			multicompilemap.variantcount *= (uint32_t)directive.size();
		}

		return;
	}

	static void ExtractStateParameters(std::string& source, ShaderStateParameters& parameters)
	{
		auto valueZWrite = StringUtilities::ExtractTokens("#ZWrite ", source, false);
		auto valueZTest = StringUtilities::ExtractTokens("#ZTest ", source, false);
		auto valueBlend = StringUtilities::ExtractTokens("#Blend ", source, false);
		auto valueColorMask = StringUtilities::ExtractTokens("#ColorMask ", source, false);
		auto valueCull = StringUtilities::ExtractTokens("#Cull ", source, false);
	
		if (!valueZWrite.empty())
		{
			valueZWrite = StringUtilities::Trim(valueZWrite);
			parameters.ZWriteEnabled = StringUtilities::ParseBool(valueZWrite, "On", "Off");
		}
	
		if (!valueZTest.empty())
		{
			parameters.ZTestEnabled = GetZTestFromString(StringUtilities::Trim(valueZTest), parameters.ZTest);
		}
	
		if (!valueBlend.empty())
		{
			auto keywords = StringUtilities::Split(valueBlend, " \n\r");
	
			if (keywords.size() == 2)
			{
				parameters.BlendEnabled = true;
				GetBlendModeFromString(keywords.at(0), parameters.BlendSrc);
				GetBlendModeFromString(keywords.at(1), parameters.BlendDst);
			}
		}
	
		GetColorMaskFromString(valueColorMask, parameters.ColorMask);
		GetCullModeFromString(StringUtilities::Trim(valueCull), parameters.CullMode, parameters.CullEnabled);
	}
	
	static void ProcessShaderVersion(std::string& source)
	{
		auto versionToken = StringUtilities::ExtractTokens("#version ", source, true);
		
		if (versionToken.empty())
		{
			source.insert(0, "#version 460\n");
			PK_CORE_LOG("Shader didn't declare language version. Declaring 460 as default.");
		}
		else
		{
			source.insert(0, versionToken);
		}
	}
	
	static void ProcessShaderTypeDefine(GLenum type, std::string& source)
	{
		switch (type)
		{
			case GL_VERTEX_SHADER: source.insert(0, "#define SHADER_STAGE_VERTEX\n"); return;
			case GL_FRAGMENT_SHADER: source.insert(0, "#define SHADER_STAGE_FRAGMENT\n"); return;
			case GL_GEOMETRY_SHADER: source.insert(0, "#define SHADER_STAGE_GEOMETRY\n"); return;
			case GL_TESS_CONTROL_SHADER: source.insert(0, "#define SHADER_STAGE_TESSELATION_CONTROL\n"); return;
			case GL_TESS_EVALUATION_SHADER: source.insert(0, "#define SHADER_STAGE_TESSELATION_EVALUATE\n"); return;
			case GL_COMPUTE_SHADER: source.insert(0, "#define SHADER_STAGE_COMPUTE\n"); return;
			default: PK_CORE_ASSERT(false, "Unknown shader type!"); return;
		}
	}
	
	static void ReadFile(const std::string& filepath, std::string& ouput)
	{
		ouput = StringUtilities::ReadFileRecursiveInclude(filepath, false);
	}
	
	static void ProcessTypeSources(const std::string& source, const std::string& sharedInclude, const std::string& variantDefines, std::unordered_map<GLenum, std::string>& shaderSources)
	{
		shaderSources.clear();

		auto typeToken = "#pragma PROGRAM_";
		auto typeTokenLength = strlen(typeToken);
		auto pos = source.find(typeToken, 0); //Start of shader type declaration line

		while (pos != std::string::npos)
		{
			auto eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			auto nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			PK_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			PK_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");

			auto begin = pos + typeTokenLength; //Start of shader type name (after "#pragma PROGRAM_" keyword)
			auto type = source.substr(begin, eol - begin);

			PK_CORE_ASSERT(GetShaderTypeFromString(type), "Invalid shader type specified");

			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[GetShaderTypeFromString(type)] = pos == std::string::npos ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}
	
		for (auto& kv : shaderSources)
		{
			kv.second.insert(0, sharedInclude);
			ProcessShaderTypeDefine(kv.first, kv.second);
			kv.second.insert(0, variantDefines);
			ProcessShaderVersion(kv.second);
		}
	}
	
	static void Compile(const std::string& filename, const std::unordered_map<GLenum, std::string>& shaderSources, std::map<uint32_t, ShaderPropertyInfo>& variablemap, GraphicsID& program)
	{
		program = glCreateProgram();
	
		PK_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");
	
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
	
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;
	
			GLuint glShader = glCreateShader(type);
	
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(glShader, 1, &sourceCStr, 0);
	
			glCompileShader(glShader);
	
			GLint isCompiled = 0;
			glGetShaderiv(glShader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(glShader, GL_INFO_LOG_LENGTH, &maxLength);
	
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(glShader, maxLength, &maxLength, &infoLog[0]);
	
				glDeleteShader(glShader);
	
				PK_CORE_LOG_HEADER("Shader (%s) Compilation Failure!", filename);
				PK_CORE_ERROR(infoLog.data());
			}
	
			glAttachShader(program, glShader);
			glShaderIDs[glShaderIDIndex++] = glShader;
		}
	
		// Link our program
		glLinkProgram(program);
	
		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
	
			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
	
			// We don't need the program anymore.
			glDeleteProgram(program);
	
			for (auto id : glShaderIDs)
			{
				glDeleteShader(id);
			}
	
			PK_CORE_ERROR("Shader link failure! \n %s", infoLog.data());
		}
	
		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	
		GLint i;
		GLint count;
		GLint size; // size of the variable
		GLenum type; // type of the variable (float, vec3 or mat4, etc)
		GLint currentSlot = 0; // Resource slot indexer
		const GLsizei bufSize = 36; // maximum name length
		GLchar name[bufSize]; // variable name in GLSL
		GLsizei length; // name length
	
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
	
		// Set as active so that texture slots can be bound
		auto currentProgram = Graphics::GetActiveShaderProgramId();

		glUseProgram(program);
	
		variablemap.clear();
	
		for (i = 0; i < count; ++i)
		{
			glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size, &type, name);
	
			PK_CORE_ASSERT(length < 32, "Uniform name (%s) exeeceds maximum name length (32)", name);
	
			for (auto j = 0; j < size; ++j)
			{
				if (size > 1)
				{
					auto istr = std::to_string(j);
					auto istrl = istr.length();
					memcpy(&name[length - 2], istr.c_str(), istrl);
					name[(int)length - 2 + (int)istrl] = ']';
					name[(int)length - 1 + (int)istrl] = '\0';
				}
	
				auto slot = GetUniformResourceSlot(currentSlot, type);
				auto location = glGetUniformLocation(program, name);
	
				// Uniform requires a resource slot.
				// Assign it & override location with the slot value.
				if (slot != -1)
				{
					glUniform1iv(location, 1, &slot);
					location = slot;
				}
	
				variablemap[StringHashID::StringToID(name)] = { location, type };
	
				// For array uniforms also map the variable name without the brackets
				if (j == 0 && size > 1)
				{
					name[StringUtilities::FirstIndexOf(name, '[')] = '\0';
					variablemap[StringHashID::StringToID(name)] = { location, type };
				}
			}
		}

		// Map Constant Buffers
		glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &count);
	
		for (i = 0; i < count; ++i)
		{
			glGetActiveUniformBlockName(program, (GLuint)i, bufSize, &length, name);
			PK_CORE_ASSERT(length < 32, "Uniform block name (%s) exeeceds maximum name length (32)", name);
			auto location = glGetUniformBlockIndex(program, name);
			glUniformBlockBinding(program, location, location);
			variablemap[StringHashID::StringToID(name)] = { (GLint)location, GL_UNIFORM_BLOCK };
		}

		glUseProgram(currentProgram);
	}
}


template<typename T> 
void AssetImporters::Import(const std::string& filepath, Ref<T>& shader)
{
	shader->m_variants.clear();

	std::string source;
	std::string sharedInclude;
	std::string variantDefines;
	std::vector<std::vector<std::string>> mckeywords;
	std::unordered_map<GLenum, std::string> shaderSources;
	std::map<uint32_t, ShaderPropertyInfo> properties;
	GraphicsID programId;

	ShaderCompiler::ReadFile(filepath, source);
	ShaderCompiler::ExtractMulticompiles(source, mckeywords, shader->m_variantMap);
	ShaderCompiler::ExtractStateParameters(source, shader->m_stateParameters);
	ShaderCompiler::GetSharedInclude(source, sharedInclude);

	for (uint32_t i = 0; i < shader->m_variantMap.variantcount; ++i)
	{
		ShaderCompiler::GetVariantDefines(mckeywords, i, variantDefines);
		ShaderCompiler::ProcessTypeSources(source, sharedInclude, variantDefines, shaderSources);
		ShaderCompiler::Compile(shader->GetFileName(), shaderSources, properties, programId);
		shader->m_variants.push_back(CreateRef<ShaderVariant>(programId, properties));
	}
}

template void AssetImporters::Import<Shader>(const std::string& filepath, Ref<Shader>& shader);