#include <Windows.h>
#include <assert.h>
#include "Graphics.h"
#include "Shader.h"

using namespace std;

#pragma region Local variables
Graphics	 graphics;

Shader*		 currentShader;
vector<int>  shaderHashIds;
unsigned int currentShaderIndex;

float shaderPreviousTime;
float shaderScaledTime;
float shaderTimeScale;

const auto hashRes	= Shader::HashId("_Resolution");
const auto hashTime	= Shader::HashId("_Time");
#pragma endregion

#pragma region Config Loading
/// <summary>
/// Helper struct for loading config data.
/// </summary>
struct Config
{
	bool	showConsole;
	int		width;
	int		height;
	float	timescale;
};

/// <summary>
/// Helper method for checking wether a string has a correct value for a bool type.
/// </summary>
static bool TryFindValidValueBool(const string& line, bool &value)
{
	auto isFalse = line.find("false") != string::npos;
	auto isTrue  = line.find("true") != string::npos;

	if (!isFalse && !isTrue)
	{
		cout << "Invalid value definition on line: " << line << endl;
		return false;
	}

	value = isTrue;
	return true;
}

/// <summary>
/// Loads a simple config file that should have values for console visibility and shader paths.
/// </summary>
Config LoadConfig(const string& filepath)
{
	ifstream file(filepath);

	if (file.fail())
	{
		cout << "Failed to open config file at: " << filepath << endl;
		return { true, 512, 512, 1.0f };
	}

	Config ret;
	string line;

	while (getline(file, line))
	{
		if (line.find("#showconsole ") != string::npos)
		{
			TryFindValidValueBool(line, ret.showConsole);
			continue;
		}

		if (line.find("#width ") != string::npos)
		{
			ret.width = stoi(line.substr(7));
			continue;
		}

		if (line.find("#height ") != string::npos)
		{
			ret.height = stoi(line.substr(8));
			continue;
		}

		if (line.find("#timescale ") != string::npos)
		{
			ret.timescale = stof(line.substr(11));
			continue;
		}
	}

	return ret;
}
#pragma endregion

#pragma region Local Methods
/// <summary>
/// Finds the next shader by namehashId and sets it to be used in upcoming drawcalls.
/// </summary>
void ShiftToNextShader()
{
	currentShader = Shader::Find(shaderHashIds.at(currentShaderIndex));
	currentShader->UseProgram();
	currentShaderIndex = (currentShaderIndex + 1) % (shaderHashIds.size());
}

/// <summary>
/// Sets the timescale used for shaders.
/// </summary>
void SetShaderTimeScale(float scale)
{
	shaderTimeScale = scale;
	cout << "Shader Timescale is: " << scale << endl;
}

/// <summary>
/// Calculates deltatime based on current timescale.
/// </summary>
void CalculateShaderTime()
{
	float currentTime	= glfwGetTime();
	float deltaTime		= currentTime - shaderPreviousTime;
	shaderPreviousTime	= currentTime;
	shaderScaledTime   += deltaTime * shaderTimeScale;
}

/// <summary>
/// Sets shader values for our current in use shader.
/// </summary>
void OnDraw(const int& width, const int& height)
{
	CalculateShaderTime();
	vec2 res = { width, height };
	currentShader->SetVector2(hashRes, res);
	currentShader->SetFloat(hashTime, shaderScaledTime);
}

/// <summary>
/// Exits application.
/// </summary>
void Exit()
{
	graphics.OnExit();
	exit(EXIT_SUCCESS);
}

/// <summary>
/// GLFW Input callback
/// </summary>
void OnInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	switch (key)
	{
		case GLFW_KEY_ESCAPE: Exit();										return;
		case GLFW_KEY_R:	  currentShader->Reimport();					return;
		case GLFW_KEY_Q:	  ShiftToNextShader();							return;
		case GLFW_KEY_W:	  SetShaderTimeScale(shaderTimeScale + 0.2f);	return;
		case GLFW_KEY_E:	  SetShaderTimeScale(shaderTimeScale - 0.2f);	return;
		default: return;
	}
}
#pragma endregion

int main(void)
{
	graphics = Graphics();

	auto config = LoadConfig("res/Config.txt");

	::ShowWindow(::GetConsoleWindow(), config.showConsole ? SW_SHOW : SW_HIDE);

	if (!graphics.TryInitialize("GL Context", config.width, config.height))
		exit(EXIT_FAILURE);

	glfwSetKeyCallback(graphics.window, OnInput);
	graphics.beforeDrawCallBacks.add<&OnDraw>();

	Shader::ImportMultiple("res/Shaders.txt");
	shaderHashIds = Shader::GetAllNameHashIds();
	ShiftToNextShader();
	SetShaderTimeScale(config.timescale);

	while (graphics.TryDraw())
	{
		glfwPollEvents();
	}

	Exit();
}