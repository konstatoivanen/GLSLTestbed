#include "PrecompiledHeader.h"
#include "Rendering/Graphics.h"
#include "Rendering/Objects/Shader.h"
#include "Rendering/RenderQueueInfo.h"
#include "Utilities/StringHashID.h"
#include "Utilities/StringUtilities.h"
#include "Utilities/Log.h"
#include <hlslmath.h>

namespace PK::Rendering::Objects
{
	using namespace PK::Utilities;
	using namespace PK::Rendering::Structs;
	using namespace PK::Math;

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
	
	uint32_t ShaderVariantMap::GetActiveIndex() const
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
			PK_CORE_LOG("%s : %s : %i", Convert::ToString(i.second.type).c_str(), StringHashID::IDToString(i.first).c_str(), i.second.location);
		}
	}
	
	void ShaderVariant::SetPropertyBlock(const ShaderPropertyBlock& propertyBlock)
	{
		for (auto& i : propertyBlock)
		{
			auto& hashId = i.first;
	
			if (m_properties.count(hashId) < 1)
			{
				continue;
			}
	
			const auto& prop = m_properties.at(hashId);
	
			if (prop.cbufferId > 0)
			{
				continue;
			}
	
			auto& info = i.second;
			uint count = info.size / Convert::Size(info.type);
			auto components = Convert::Components(info.type);
	
			switch (info.type)
			{
				case CG_TYPE::FLOAT: glUniform1fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::FLOAT2: glUniform2fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::FLOAT3: glUniform3fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::FLOAT4: glUniform4fv(prop.location, count, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::FLOAT2X2: glUniformMatrix2fv(prop.location, count, GL_FALSE, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::FLOAT3X3: glUniformMatrix3fv(prop.location, count, GL_FALSE, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::FLOAT4X4: glUniformMatrix4fv(prop.location, count, GL_FALSE, propertyBlock.GetElementPtr<float>(info)); break;
				case CG_TYPE::INT:  glUniform1iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
				case CG_TYPE::INT2: glUniform2iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
				case CG_TYPE::INT3: glUniform3iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
				case CG_TYPE::INT4: glUniform4iv(prop.location, count, propertyBlock.GetElementPtr<int>(info)); break;
				case CG_TYPE::UINT:  glUniform1uiv(prop.location, count, propertyBlock.GetElementPtr<uint>(info)); break;
				case CG_TYPE::UINT2: glUniform2uiv(prop.location, count, propertyBlock.GetElementPtr<uint>(info)); break;
				case CG_TYPE::UINT3: glUniform3uiv(prop.location, count, propertyBlock.GetElementPtr<uint>(info)); break;
				case CG_TYPE::UINT4: glUniform4uiv(prop.location, count, propertyBlock.GetElementPtr<uint>(info)); break;
				case CG_TYPE::HANDLE: glUniformHandleui64vARB(prop.location, count, propertyBlock.GetElementPtr<ulong>(info)); break;
				case CG_TYPE::TEXTURE: GraphicsAPI::BindTextures(prop.location, propertyBlock.GetElementPtr<GraphicsID>(info), count); break;
				case CG_TYPE::IMAGE_PARAMS: GraphicsAPI::BindImages(prop.location, propertyBlock.GetElementPtr<ImageBindDescriptor>(info), count); break;
				case CG_TYPE::CONSTANT_BUFFER: GraphicsAPI::BindBuffers(CG_TYPE::CONSTANT_BUFFER, prop.location, propertyBlock.GetElementPtr<GraphicsID>(info), count); break;
				case CG_TYPE::COMPUTE_BUFFER: GraphicsAPI::BindBuffers(CG_TYPE::COMPUTE_BUFFER, prop.location, propertyBlock.GetElementPtr<GraphicsID>(info), count); break;
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
		static bool IsSlottedType(GLenum variableType)
		{
			// Add support for more types later
			switch (variableType)
			{
				case GL_SAMPLER_1D:
				case GL_SAMPLER_1D_ARRAY:
				case GL_SAMPLER_2D:
				case GL_SAMPLER_2D_ARRAY:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_CUBE:
				case GL_SAMPLER_CUBE_MAP_ARRAY:
				case GL_IMAGE_1D:
				case GL_IMAGE_1D_ARRAY:
				case GL_IMAGE_2D:
				case GL_IMAGE_2D_ARRAY:
				case GL_IMAGE_3D:
					return true;
			}

			return false;
		}

		static GLint GetUniformSamplerSlot(const std::map<GLint, GLint>& explicitbindings, GLint location, GLint& currentSlot, GLenum variableType)
		{
			if (IsSlottedType(variableType))
			{
				if (explicitbindings.count(location))
				{
					return explicitbindings.at(location);
				}

				collisioncheck:
				for (auto& kv : explicitbindings)
				{
					if (kv.second == currentSlot)
					{
						++currentSlot;
						goto collisioncheck;
					}
				}

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
		
			if (type == "COMPUTE")
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
			else if (ztest == "Equal")
			{
				mode = GL_EQUAL;
				return true;
			}
			
			PK_CORE_ASSERT(false, "Invalid Argument type for ZTest value");
			return false;
		}
		
		static bool IsInterfacePresentInProgramStages(GLuint program, GLuint programInterface, GLuint index)
		{
			GLuint props[6] =
			{
				GL_REFERENCED_BY_FRAGMENT_SHADER,
				GL_REFERENCED_BY_VERTEX_SHADER,
				GL_REFERENCED_BY_TESS_CONTROL_SHADER,
				GL_REFERENCED_BY_TESS_EVALUATION_SHADER,
				GL_REFERENCED_BY_GEOMETRY_SHADER,
				GL_REFERENCED_BY_COMPUTE_SHADER
			};

			GLint count;

			for (uint i = 0; i < 6; ++i)
			{
				glGetProgramResourceiv(program, programInterface, index, 1, props + i, 1, NULL, &count);
			
				if (count)
				{
					return true;
				}
			}

			return false;
		}

		static void GetCullModeFromString(const std::string& cull, GLenum& mode, bool& enabled)
		{
			if (cull.empty() || cull == "Off")
			{
				mode = GL_FALSE;
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
				pos = Utilities::String::ExtractToken(pos, "#multi_compile ", source, output, false);
	
				if (pos == std::string::npos)
				{
					break;
				}
	
				keywords.push_back(Utilities::String::Split(output, " "));
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
	
		static void ExtractInstancingInfo(const std::string& source, const ShaderVariantMap& variants, ShaderInstancingInfo& instancingInfo)
		{
			auto instancingKeyword = StringHashID::StringToID("PK_ENABLE_INSTANCING");
			instancingInfo.supportsInstancing = variants.keywords.count(instancingKeyword) > 0;
			instancingInfo.hasInstancedProperties = false;

			if (!instancingInfo.supportsInstancing)
			{
				return;
			}

			std::vector<BufferElement> elements;
			std::vector<std::string> instancedProperties;
			Utilities::String::FindTokens("PK_INSTANCED_PROPERTY", source, instancedProperties, true);

			for (auto& propRaw : instancedProperties)
			{
				auto values = Utilities::String::Split(propRaw, " ;\n\r");

				if (values.size() != 3)
				{
					continue;
				}
			
				auto type = Math::Convert::FromUniformString(values.at(1).c_str());
				auto name = values.at(2);

				elements.push_back(BufferElement(type, name));
			}

			instancingInfo.hasInstancedProperties = !elements.empty();
		
			if (instancingInfo.hasInstancedProperties)
			{
				instancingInfo.propertyLayout = BufferLayout(elements);
			}
		}

		static void ExtractStateAttributes(std::string& source, FixedStateAttributes& parameters)
		{
			auto valueZWrite = Utilities::String::ExtractToken("#ZWrite ", source, false);
			auto valueZTest = Utilities::String::ExtractToken("#ZTest ", source, false);
			auto valueBlend = Utilities::String::ExtractToken("#Blend ", source, false);
			auto valueColorMask = Utilities::String::ExtractToken("#ColorMask ", source, false);
			auto valueCull = Utilities::String::ExtractToken("#Cull ", source, false);
		
			if (!valueZWrite.empty())
			{
				valueZWrite = Utilities::String::Trim(valueZWrite);
				parameters.ZWriteEnabled = Utilities::String::ParseBool(valueZWrite, "On", "Off");
			}
		
			if (!valueZTest.empty())
			{
				parameters.ZTestEnabled = GetZTestFromString(Utilities::String::Trim(valueZTest), parameters.ZTest);
			}
		
			if (!valueBlend.empty())
			{
				auto keywords = Utilities::String::Split(valueBlend, " \n\r");
		
				if (keywords.size() == 2)
				{
					parameters.BlendEnabled = true;
					GetBlendModeFromString(keywords.at(0), parameters.Blend.Source);
					GetBlendModeFromString(keywords.at(1), parameters.Blend.Destination);
				}
			}
		
			GetColorMaskFromString(valueColorMask, parameters.ColorMask);
			GetCullModeFromString(Utilities::String::Trim(valueCull), parameters.CullMode, parameters.CullEnabled);
		}
		
		static void ExtractRenderQueueIndex(std::string& source, uint32_t* queueIndex)
		{
			auto valueIdentifier = Utilities::String::ExtractToken("#RenderPass", source, false);

			if (!valueIdentifier.empty())
			{
				*queueIndex = RenderQueueInfo::Get()->GetQueueIndex(valueIdentifier);
			}
			else
			{
				*queueIndex = 0;
			}
		}

		static void ProcessShaderVersion(std::string& source)
		{
			auto versionToken = Utilities::String::ExtractToken("#version ", source, true);
			
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
			ouput = Utilities::String::ReadFileRecursiveInclude(filepath);
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
			variablemap.clear();
		
			PK_CORE_ASSERT(shaderSources.size() > 0, "No shader sources supplied for %s", filename.c_str());
		
			auto stageCount = shaderSources.size();
	
			GLenum* glShaderIDs = PK_STACK_ALLOC(GLenum, stageCount);
			int glShaderIDIndex = 0;
		
			for (auto& kv : shaderSources)
			{
				auto type = kv.first;
				const auto& source = kv.second;
		
				auto glShader = glCreateShader(type);
				
				const auto* sourceCStr = source.c_str();
		
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
		
					PK_CORE_LOG_HEADER("Shader (%s) Compilation Failure!", filename.c_str());
					PK_CORE_ERROR(infoLog.data());
				}
		
				glAttachShader(program, glShader);
				glShaderIDs[glShaderIDIndex++] = glShader;
			}
		
			glLinkProgram(program);
		
			GLint isLinked = 0;
			glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	
			if (isLinked == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
	
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		
				glDeleteProgram(program);
		
				for (auto i = 0; i < stageCount; ++i)
				{
					glDeleteShader(glShaderIDs[i]);
				}
		
				PK_CORE_ERROR("Shader link failure! \n%s \n%s", filename.c_str(), infoLog.data());
			}
		
			for (auto i = 0; i < stageCount; ++i)
			{
				glDetachShader(program, glShaderIDs[i]);
				glDeleteShader(glShaderIDs[i]);
			}
		
			// Set as active so that texture slots can be bound
			auto currentProgram = GraphicsAPI::GetActiveShaderProgramId();
			glUseProgram(program);
		
			GLint uniformCount;
			GLint blockCount;
			GLint bufferCount;
	
			GLint blockIndex; // Index of the cbuffer the variable belongs to
			GLint size; // size of the variable
			GLenum type; // type of the variable (float, vec3 or mat4, etc)
			GLint samplerSlot = 0; // Resource slot indexer
	
			GLsizei maxnamelength = 0; // maximum name length
			GLsizei maxnamelengtht = 0; // maximum name length temp
			GLsizei length; // name length
	
			std::map<GLint, GLint> explicitBindings;

			glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_MAX_NAME_LENGTH, &maxnamelengtht);
			maxnamelength = glm::max(maxnamelength, maxnamelengtht);

			glGetProgramInterfaceiv(program, GL_UNIFORM, GL_MAX_NAME_LENGTH, &maxnamelengtht);
			maxnamelength = glm::max(maxnamelength, maxnamelengtht);

			glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxnamelengtht);
			maxnamelength = glm::max(maxnamelength, maxnamelengtht);

			glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &blockCount);
			glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);
			glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &bufferCount);
		
			maxnamelength += 4;
			auto* name = PK_STACK_ALLOC(char, maxnamelength);
	
			// Map CBuffers
			for (GLuint i = 0; i < (GLuint)blockCount; ++i)
			{
				glGetProgramResourceName(program, GL_UNIFORM_BLOCK, i, maxnamelength, &length, name);
				auto location = glGetProgramResourceIndex(program, GL_UNIFORM_BLOCK, name);
				glUniformBlockBinding(program, location, location);
				variablemap[StringHashID::StringToID(name)] = { (ushort)location, CG_TYPE::CONSTANT_BUFFER, 0, false };
			}
	
			// Map Compute Buffers
			for (GLuint i = 0; i < (GLuint)bufferCount; ++i)
			{
				// Inteface query returns false positives for ssbos but GL_REFERENCED_BY_*_SHADER doesn't ¯\_(ツ)_/¯
				if (!IsInterfacePresentInProgramStages(program, GL_SHADER_STORAGE_BLOCK, i))
				{
					continue;
				}
				
				glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, maxnamelength, &length, name);
				auto location = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name);
				glShaderStorageBlockBinding(program, location, location);
				variablemap[StringHashID::StringToID(name)] = { (ushort)location, CG_TYPE::COMPUTE_BUFFER, 0, false };
			}
	
			// Map explicit layout binding variables
			for (GLuint i = 0; i < (GLuint)uniformCount; ++i)
			{
				glGetActiveUniform(program, i, maxnamelength, &length, &size, &type, name);

				if (!IsSlottedType(type) || size != 1)
				{
					continue;
				}

				auto location = glGetUniformLocation(program, name);
				
				if (location < 0)
				{
					continue;
				}

				GLint explicitSlot;
				glGetUniformiv(program, location, &explicitSlot);

				if (explicitSlot > 0)
				{
					explicitBindings[location] = explicitSlot;
				}
			}

			// Map regular uniforms
			for (GLuint i = 0; i < (GLuint)uniformCount; ++i)
			{
				glGetActiveUniform(program, i, maxnamelength, &length, &size, &type, name);
				glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
				auto cgtype = Convert::FromUniformType(type);
		
				for (auto j = 0; j < size; ++j)
				{
					if (size > 1 && j > 0)
					{
						auto istr = std::to_string(j);
						auto istrl = istr.length();
						memcpy(&name[length - 2], istr.c_str(), istrl);
						name[(int)length - 2 + (int)istrl] = ']';
						name[(int)length - 1 + (int)istrl] = '\0';
					}
		
					auto location = glGetUniformLocation(program, name);
					auto slot = GetUniformSamplerSlot(explicitBindings, location, samplerSlot, type);
		
					// Uniform requires a resource slot.
					// Assign it & override location with the slot value.
					if (slot != -1)
					{
						glUniform1iv(location, 1, &slot);
						location = slot;
					}
		
					variablemap[StringHashID::StringToID(name)] = { (ushort)location, cgtype, (byte)(blockIndex + 1), false };
		
					// For array uniforms also map the variable name without the brackets
					if (j == 0 && size > 1)
					{
						name[(int)length - 3] = '\0';
						variablemap[StringHashID::StringToID(name)] = { (ushort)location, cgtype, (byte)(blockIndex + 1), false };
						name[(int)length - 3] = '[';
					}
				}
			}
	
			glUseProgram(currentProgram);
		}
	}
}

template<> 
void PK::Core::AssetImporters::Import(const std::string& filepath, PK::Utilities::Ref<PK::Rendering::Objects::Shader>& shader)
{
	shader->m_variants.clear();

	// A lot of hacky stuff in this parser at the moment.
	// @TODO Consider clean up once priorities allow it.
	std::string source;
	std::string sharedInclude;
	std::string variantDefines;
	std::vector<std::vector<std::string>> mckeywords;
	std::unordered_map<GLenum, std::string> shaderSources;
	std::map<uint32_t, PK::Rendering::Structs::ShaderPropertyInfo> properties;
	PK::Rendering::Objects::GraphicsID programId;

	PK::Rendering::Objects::ShaderCompiler::ReadFile(filepath, source);
	PK::Rendering::Objects::ShaderCompiler::ExtractMulticompiles(source, mckeywords, shader->m_variantMap);
	PK::Rendering::Objects::ShaderCompiler::ExtractStateAttributes(source, shader->m_stateAttributes);
	PK::Rendering::Objects::ShaderCompiler::ExtractRenderQueueIndex(source, &shader->m_renderQueueIndex);
	PK::Rendering::Objects::ShaderCompiler::ExtractInstancingInfo(source, shader->m_variantMap, shader->m_instancingInfo);

	PK::Rendering::Objects::ShaderCompiler::GetSharedInclude(source, sharedInclude);

	for (uint32_t i = 0; i < shader->m_variantMap.variantcount; ++i)
	{
		PK::Rendering::Objects::ShaderCompiler::GetVariantDefines(mckeywords, i, variantDefines);
		PK::Rendering::Objects::ShaderCompiler::ProcessTypeSources(source, sharedInclude, variantDefines, shaderSources);
		PK::Rendering::Objects::ShaderCompiler::Compile(shader->GetFileName(), shaderSources, properties, programId);
		shader->m_variants.push_back(CreateRef<PK::Rendering::Objects::ShaderVariant>(programId, properties));
	}
}