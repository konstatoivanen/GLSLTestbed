#include <tuple>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include "Graphics.h"
#include "Shader.h"

#pragma region Local
hash<string>	  hashStringToId;
map<int, Shader*> shaderCollection;

enum class ShaderType
{
	None	 = -1,
	Vertex	 =  0, 
	Fragment =  1
};

static tuple<string, string, string, vector<string>> ParseShader(const string& filepath)
{
	ifstream file(filepath);

	vector<string> uniforms;

	if (file.fail())
	{
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
#pragma endregion

#pragma region Public Methods
Shader::Shader(const unsigned int& sprogramId, const vector<string>& uniforms)
{
	this->programId = sprogramId;
	
	auto size = uniforms.size();

	for (int i = 0; i < size; i++)
	{
		auto uniform		= uniforms.at(i);
		auto hashId			= HashId(uniform);
		variableMap[hashId] = glGetUniformLocation(programId, uniform.c_str());
	}
}

Shader* Shader::ImportShader(const string& filepath)
{
	auto source  = ParseShader(filepath);
	auto program = glCreateProgram();

	auto vs	= CompileShader(GL_VERTEX_SHADER,   get<0>(source));
	auto fs	= CompileShader(GL_FRAGMENT_SHADER, get<1>(source));

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	auto name	   = get<2>(source);
	auto hashId	   = HashId(name);
	auto newShader = new Shader(program, get<3>(source));

	shaderCollection[hashId] = newShader;

	cout << (name.empty()? "Shader Not Found At: " + filepath : "Compiled Shader: " + name) << endl;

	return newShader;
}
Shader* Shader::Find(const string& name)
{
	auto hashId = HashId(name);
	return shaderCollection[hashId];
}
Shader* Shader::Find(const int& hashId)
{
	return shaderCollection[hashId];
}
void	Shader::ReleaseAll()
{
	for (auto const& i : shaderCollection)
	{
		glDeleteProgram(i.second->programId);
		delete i.second;
	}

	shaderCollection.clear();
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