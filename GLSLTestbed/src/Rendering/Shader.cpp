#include "PrecompiledHeader.h"
#include "Rendering/Graphics.h"
#include "Rendering/Shader.h"
#include "Utilities/StringHashID.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/Log.h"
#include <ext.hpp>

Shader::~Shader()
{
	glDeleteProgram(m_graphicsId);
}

void Shader::ListUniforms()
{
	PK_CORE_LOG_HEADER("Listing uniforms for shader: %s", m_name.c_str());

	for (auto& i : m_variableMap)
	{
		PK_CORE_LOG("%s : %i", StringHashID::IDToString(i.first).c_str(), i.second.location);
	}
}

void Shader::SetPropertyBlock(const ShaderPropertyBlock& propertyBlock)
{
	for (auto& i : propertyBlock)
	{
		auto hashId = i.first;

		if (!HasProperty(hashId))
		{
			continue;
		}

		auto& prop = GetProperty(hashId);
		auto& info = i.second;
		auto count = info.size / CGType::Size(info.type);
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
				for (uint32_t i = 0; i < count; ++i)
				{
					glBindTextureUnit(prop.location + i, textureIds[i]);
				}
				break;
			}
			default: PK_CORE_ERROR("Invalid Shader Property Type"); break;
		}
	}
}


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

static GLenum GetBlendModeFromString(const std::string& blendMode)
{
	if (blendMode == "Zero")
	{
		return GL_ZERO;
	}
	else if (blendMode == "One")
	{
		return GL_ONE;
	}
	else if (blendMode == "SrcColor")
	{
		return GL_SRC_COLOR;
	}
	else if (blendMode == "OneMinusSrcColor")
	{
		return GL_ONE_MINUS_SRC_COLOR;
	}
	else if (blendMode == "DstColor")
	{
		return GL_DST_COLOR;
	}
	else if (blendMode == "OneMinusDstColor")
	{
		return GL_ONE_MINUS_DST_COLOR;
	}
	else if (blendMode == "SrcAlpha")
	{
		return GL_SRC_ALPHA;
	}
	else if (blendMode == "OneMinusSrcAlpha")
	{
		return GL_ONE_MINUS_SRC_ALPHA;
	}
	else if (blendMode == "DstAlpha")
	{
		return GL_DST_ALPHA;
	}
	else if (blendMode == "OneMinusDstAlpha")
	{
		return GL_ONE_MINUS_DST_ALPHA;
	}
	else if (blendMode == "ConstColor")
	{
		return GL_CONSTANT_COLOR;
	}
	else if (blendMode == "OneMinusConstColor")
	{
		return GL_ONE_MINUS_CONSTANT_COLOR;
	}
	else if (blendMode == "ConstAlpha")
	{
		return GL_CONSTANT_ALPHA;
	}
	else if (blendMode == "OneMinusConstAlpha")
	{
		return GL_ONE_MINUS_CONSTANT_ALPHA;
	}

	PK_CORE_ASSERT(false, "Unknown blend mode!");
	return 0;
}

static GLbyte GetColorMaskFromString(const std::string& colorMask)
{
	GLbyte mask = 0;

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

	return mask;
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

static bool GetCullModeFromString(const std::string& cull, GLenum& mode)
{
	if (cull == "Off") 
	{
		mode = GL_BACK;
		return false;
	}
	else if (cull == "Front") 
	{
		mode = GL_FRONT;
		return true;
	}
	else if (cull == "Back") 
	{
		mode = GL_BACK;
		return true;
	}

	PK_CORE_ASSERT(false, "Invalid Argument type for Cull value");
	return false;
}

static std::string GetSharedInclude(const std::string& source)
{
	auto typeToken = "#pragma PROGRAM_";
	auto typeTokenLength = strlen(typeToken);
	auto pos = source.find(typeToken, 0); //Start of shader type declaration line
	std::string sharedInclude = "";

	// Treat code in the beginning of the source as shared include
	if (pos != std::string::npos && pos != 0)
	{
		sharedInclude = source.substr(0, pos);
	}

	return sharedInclude;
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
			parameters.BlendSrc = GetBlendModeFromString(keywords.at(0));
			parameters.BlendDst = GetBlendModeFromString(keywords.at(1));
		}
	}

	parameters.ColorMask = 255;

	if (!valueColorMask.empty())
	{
		parameters.ColorMask = GetColorMaskFromString(valueColorMask);
	}

	if (!valueCull.empty())
	{
		parameters.CullEnabled = GetCullModeFromString(StringUtilities::Trim(valueCull), parameters.CullMode);
	}
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

static void ProcessShaderTypeSources(const std::string& source, std::unordered_map<GLenum, std::string>& shaderSources)
{
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
}


std::string ShaderCompiler::ReadFile(const std::string& filepath)
{
	return StringUtilities::ReadFileRecursiveInclude(filepath, false);

	/*
	std::string result;
	std::ifstream input(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII

	if (input)
	{
		input.seekg(0, std::ios::end);
		size_t size = input.tellg();
		if (size != -1)
		{
			result.resize(size);
			input.seekg(0, std::ios::beg);
			input.read(&result[0], size);
		}
		else
		{
			PK_CORE_ERROR("Could not read from file '%s'", filepath.c_str());
		}
	}
	else
	{
		PK_CORE_ERROR("Could not open file '%s'", filepath.c_str());
	}

	return result;
	*/
}

std::unordered_map<GLenum, std::string> ShaderCompiler::Preprocess(std::string& source, ShaderStateParameters& parameters)
{
	ExtractStateParameters(source, parameters);

	std::unordered_map<GLenum, std::string> shaderSources;

	auto sharedInclude = GetSharedInclude(source);
	
	ProcessShaderTypeSources(source, shaderSources);

	for (auto& kv : shaderSources)
	{
		kv.second.insert(0, sharedInclude);
		ProcessShaderTypeDefine(kv.first, kv.second);
		ProcessShaderVersion(kv.second);
	}

	return shaderSources;
}

void ShaderCompiler::Compile(Shader* shader, const std::unordered_map<GLenum, std::string>& shaderSources)
{
	auto program = glCreateProgram();

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

			PK_CORE_LOG_HEADER("Shader (%s) Compilation Failure!", shader->m_name.c_str());
			PK_CORE_ERROR(infoLog.data());
			break;
		}

		glAttachShader(program, glShader);
		glShaderIDs[glShaderIDIndex++] = glShader;
	}

	shader->m_graphicsId = program;

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

		PK_CORE_ERROR(infoLog.data());
		PK_CORE_ASSERT(false, "Shader link failure!");
		return;
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

	shader->m_variableMap.clear();

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

			shader->m_variableMap[StringHashID::StringToID(name)] = { location, type };

			// For array uniforms also map the variable name without the brackets
			if (j == 0 && size > 1)
			{
				name[StringUtilities::FirstIndexOf(name, '[')] = '\0';
				shader->m_variableMap[StringHashID::StringToID(name)] = { location, type };
			}
		}
	}

	glUseProgram(currentProgram);
}


Ref<Shader> ShaderCollection::Import(const std::string& filepath)
{
	auto name = StringUtilities::ReadFileName(filepath);
	auto hashId = StringHashID::StringToID(name);
	auto shader = GetShader(hashId);
	auto source = ShaderCompiler::ReadFile(filepath);
	auto shaderSources = ShaderCompiler::Preprocess(source, shader->m_stateParameters);

	shader->m_name = name;
	shader->m_cachedFilepath = filepath;

	ShaderCompiler::Compile(shader.get(), shaderSources);

	m_shaderProgramMap[shader->m_graphicsId] = shader->m_hashId;

	return shader;
}

void ShaderCollection::Reimport(Ref<Shader> shader)
{
	bool inUse = Graphics::GetActiveShaderProgramId() == shader->m_graphicsId;

	glDeleteProgram(shader->m_graphicsId);

	Import(shader->m_cachedFilepath);

	if (inUse)
	{
		Graphics::SetPass(shader);
	}
}

void ShaderCollection::ImportCollection(const std::string& filepath)
{
	std::ifstream file(filepath);

	PK_CORE_ASSERT(!file.fail(), "Failed to open shader list file at: %s", filepath.c_str());

	std::string line;

	while (std::getline(file, line))
	{
		Import(line);
	}
}

void ShaderCollection::Release()
{
	m_shaderCollection.clear();
}


Ref<Shader> ShaderCollection::GetShader(uint32_t hashId)
{
	if (m_shaderCollection.count(hashId) > 0)
	{
		return m_shaderCollection[hashId];
	}
	
	auto shader = CreateRef<Shader>();
	shader->m_hashId = hashId;
	m_shaderCollection[hashId] = shader;
	return shader;
}

Ref<Shader>	ShaderCollection::Find(const std::string& name)
{
	return ShaderCollection::Find(StringHashID::StringToID(name));
}

Ref<Shader>	ShaderCollection::Find(uint32_t hashId)
{
	return m_shaderCollection.count(hashId) > 0 ? m_shaderCollection.at(hashId) : nullptr;
}

bool ShaderCollection::FindActive(Ref<Shader>& shader)
{
	auto currentProgram = Graphics::GetActiveShaderProgramId();

	if (m_shaderProgramMap.count(currentProgram) > 0)
	{
		shader = m_shaderCollection[m_shaderProgramMap[currentProgram]];
		return true;
	}

	return false;
}

std::vector<uint32_t> ShaderCollection::GetAllShaderHashIds()
{
	std::vector<uint32_t> ret;

	for (auto const& i : m_shaderCollection)
	{
		ret.push_back(i.first);
	}

	return ret;
}