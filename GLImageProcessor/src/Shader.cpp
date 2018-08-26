#include <tuple>
#include <string>
#include <functional>
#include "Graphics.h"
#include "Shader.h"

#pragma region Local
//Using hash to generate ids from strings can cause collisions but for now this is an easy solution.
hash<string>	  hashStringToId;
map<int, Shader*> shaderCollection;

typedef tuple<string, string, string, vector<string>> ParsedShader;

enum class ShaderType
{
	None	 = -1,
	Vertex	 =  0, 
	Fragment =  1
};

/// <summary>
/// Parses shader into:
/// Vertex program
/// Fragment program
/// Name
/// Uniform variable names
/// </summary>
static ParsedShader ParseShader(const string& filepath)
{
	ifstream file(filepath);

	vector<string> uniforms;

	if (file.fail())
	{
		cout << "Failed to open shader file at: " << filepath << endl;
		return {"","","", uniforms};
	}

	string		 line;
	stringstream composite[3];
	auto		 type = ShaderType::None;

	while (getline(file, line))
	{
		if (line.find("#pragma name ") != string::npos)
		{
			composite[2] << line.substr(13);
			continue;
		}

		if (line.find("#pragma vertex") != string::npos)
		{
			type = ShaderType::Vertex;
			continue;
		}

		if (line.find("#pragma fragment") != string::npos)
		{
			type = ShaderType::Fragment;
			continue;
		}
		
		if (line.find("#pragma variable ") != string::npos)
		{
			uniforms.push_back(line.substr(17));
			continue;
		}

		if (type == ShaderType::None)
		{
			continue;
		}
			
		composite[(int)type] << line << '\n';
	}

	return make_tuple(composite[0].str(), composite[1].str(), composite[2].str(), uniforms);
}
/// <summary>
/// Submits a raw shader string for OpenGL to compile 
/// and returns the resultant program id.
/// </summary>
static unsigned int CompileShader(unsigned int type, const string& source)
{
	auto id = glCreateShader(type);
	const char* src = source.c_str();

	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* msg = (char*)alloca(length * sizeof(char));

		glGetShaderInfoLog(id, length, &length, msg);
		cout << "SHADER COMPILATION FAILURE! IN " << (type == GL_VERTEX_SHADER ? "VERTEX: " : "FRAGMENT: ") << msg << endl;
	}

	return id;
}
/// <summary>
/// Compiles and creates shader in the OpenGL contex
/// and returns the resultant programId
/// </summary>
static unsigned int CreateShaderProgram(const string& vert, const string& frag)
{
	auto program = glCreateProgram();

	auto vs = CompileShader(GL_VERTEX_SHADER, vert);
	auto fs = CompileShader(GL_FRAGMENT_SHADER, frag);

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}
#pragma endregion

#pragma region Public Methods
Shader::Shader(const unsigned int& programId, const vector<string>& uniforms, const string filepath)
{
	this->programId			= programId;
	this->cachedFilepath	= filepath;
	this->MapVariables(uniforms);
}

Shader*		Shader::Import(const string& filepath)
{
	auto source  = ParseShader(filepath);
	auto name	 = get<2>(source);
	auto hashId	 = HashId(name);

	if (shaderCollection.count(hashId) > 0)
	{
		cout << "Shader is already imported: " << name << endl;
		return shaderCollection[hashId];
	}

	cout << (name.empty()? "Shader does not include a name definition: " + filepath : "Imported shader: " + name) << endl;

	auto program			 = CreateShaderProgram(get<0>(source), get<1>(source));
	auto newShader			 = new Shader(program, get<3>(source), filepath);
	shaderCollection[hashId] = newShader;

	return newShader;
}
void		Shader::ImportMultiple(const string& filepath)
{
	ifstream file(filepath);

	if (file.fail())
	{
		cout << "Failed to open shader list file at: " << filepath << endl;
		return;
	}

	string line;
	while (getline(file, line)) Shader::Import(line);
}
void		Shader::ReleaseAll()
{
	for (auto const& i : shaderCollection)
	{
		glDeleteProgram(i.second->programId);
		delete i.second;
	}

	shaderCollection.clear();
}
Shader*		Shader::Find(const string& name)
{
	auto hashId = HashId(name);
	return shaderCollection[hashId];
}
Shader*		Shader::Find(const int& hashId)
{
	return shaderCollection[hashId];
}
vector<int> Shader::GetAllNameHashIds()
{
	vector<int> ret;

	for (auto const& i : shaderCollection)
		ret.push_back(i.first);

	return ret;
}

void Shader::MapVariables(const vector<string>& uniforms)
{
	variableMap.clear();
	auto size = uniforms.size();
	for (int i = 0; i < size; ++i)
	{
		auto uniform		= uniforms.at(i);
		auto hashId			= HashId(uniform);
		variableMap[hashId] = glGetUniformLocation(programId, uniform.c_str());
	}
}
void Shader::Reimport()
{
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

	bool inUse = currentProgram == programId;

	glDeleteProgram(programId);

	auto source = ParseShader(cachedFilepath);
	auto name	= get<2>(source);
	auto hashId = HashId(name);

	cout << (name.empty() ? "Shader does not include a name definition: " + cachedFilepath : "Reimported shader: " + name) << endl;

	programId = CreateShaderProgram(get<0>(source), get<1>(source));
	MapVariables(get<3>(source));

	if (inUse) UseProgram();
}
int  Shader::HashId(const string& name)
{
	return hashStringToId(name);
}
void Shader::SetVector4(const int& hashId, vec4 value)
{
	int vloc = variableMap[hashId];
	glUniform4f(vloc, value[0], value[1], value[2], value[3]);
}
void Shader::SetVector3(const int& hashId, vec3 value)
{
	int vloc = variableMap[hashId];
	glUniform3f(vloc, value[0], value[1], value[2]);
}
void Shader::SetVector2(const int& hashId, vec2 value)
{
	int vloc = variableMap[hashId];
	glUniform2f(vloc, value[0], value[1]);
}
void Shader::SetFloat(const int& hashId, float value)
{
	int vloc = variableMap[hashId];
	glUniform1f(vloc, value);
}
#pragma endregion