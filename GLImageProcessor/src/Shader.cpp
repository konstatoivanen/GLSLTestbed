#include "Graphics.h"
#include "Shader.h"
#include <tuple>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <map>;


#pragma region Local
hash<string>			hashStringToId;
map<int, unsigned int>	shaderCollection;

enum class ShaderType
{
	None = -1,
	Vertex = 0, 
	Fragment = 1
};

static tuple<string, string, string> ParseShader(const string& filepath)
{
	ifstream	 file(filepath);

	if (file.fail())
	{
		return {"","",""};
	}

	string		 name;
	string		 line;
	stringstream composite[2];
	auto		 type = ShaderType::None;

	while (getline(file, line))
	{
		if (line.find("#pragma name ") != string::npos)
		{
			name = line.substr(13);
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
			
		composite[(int)type] << line << '\n';
	}

	return make_tuple(composite[0].str(), composite[1].str(), name);
}

static unsigned int CompileShader(unsigned int type, const string& source)
{
	unsigned int id = glCreateShader(type);
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
		cout << "SHADER COMPILATION FAILURE!" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << msg << endl;
	}

	return id;
}
#pragma endregion


#pragma region Public Methods
unsigned int Shader::LoadShader(const string& filepath)
{
	auto source = ParseShader(filepath);

	unsigned int program = glCreateProgram();

	unsigned int vs = CompileShader(GL_VERTEX_SHADER,   get<0>(source));
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, get<1>(source));

	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	auto name   = get<2>(source);
	auto hashId = HashId(name);
	shaderCollection[hashId] = program;

	cout << (name.empty()? "Shader Not Found At: " + filepath : "Compiled Shader: " + name) << endl;

	return program;
}

unsigned int Shader::GetProgram(const string& name)
{
	auto hashId = hashStringToId(name);
	return shaderCollection[hashId];
}

unsigned int Shader::GetProgram(int hashId)
{
	return shaderCollection[hashId];
}

int Shader::HashId(const string& name)
{
	return hashStringToId(name);
}
#pragma endregion