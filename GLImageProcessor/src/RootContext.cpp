#include <Windows.h>
#include <assert.h>
#include "Graphics.h"
#include "Shader.h"

using namespace std;

Graphics graphics;

///forward declare exit for it to be used in shader engine.
///@TODO pass graphics by reference since global variables are dirty.
void Exit();

struct Config
{
	bool	showConsole;
	int		width;
	int		height;
	float	timescale;

	/// <summary>
	/// Helper method for checking wether a string has a correct value for a bool type.
	/// </summary>
	static bool TryFindValidValueBool(const string& line, bool &value)
	{
		auto isFalse = line.find("false") != string::npos;
		auto isTrue = line.find("true") != string::npos;

		if (!isFalse && !isTrue)
		{
			cout << "Invalid value definition on line: " << line << endl;
			return false;
		}

		value = isTrue;
		return true;
	}

	///<summary>
	/// load values from file and construct config struct
	///</summary>
	Config(const string& filepath)
	{
		ifstream file(filepath);

		if (file.fail())
		{
			cout << "Failed to open config file at: " << filepath << endl;
			return;
		}

		string line;

		while (getline(file, line))
		{
			if (line.find("#showconsole ") != string::npos)
			{
				TryFindValidValueBool(line, showConsole);
				continue;
			}

			if (line.find("#width ") != string::npos)
			{
				width = stoi(line.substr(7));
				continue;
			}

			if (line.find("#height ") != string::npos)
			{
				height = stoi(line.substr(8));
				continue;
			}

			if (line.find("#timescale ") != string::npos)
			{
				timescale = stof(line.substr(11));
				continue;
			}
		}
	}
};

///<summary>
/// An engine that supplies time and resolution for current shaders
/// and performs actions based on glfw input.
/// Q: next shader, W: timescale -= 0.2, E: timescale += 0.2, R: reload current shader, Escape: Exit application.
///</summary>
class ShaderEngine : public ShaderModifier, public InputReceiver
{
	public:
		virtual ~ShaderEngine()
		{

		}
		/// <summary>
		/// Sets shader values for our current in use shader.
		/// </summary>
		virtual void OnDraw(int width, int height)
		{
			CalculateTime();
			vec2 res = { width, height };
			currentShader->SetVector2(hashRes, res);
			currentShader->SetFloat(hashTime, scaledTime);
		}
		/// <summary>
		/// GLFW Input callback
		/// </summary>
		virtual void OnInput(int key, int scancode, int action, int mods)
		{
			if (action != GLFW_PRESS)
				return;

			switch (key)
			{
				case GLFW_KEY_ESCAPE: Exit();							return;
				case GLFW_KEY_R:	  currentShader->Reimport();		return;
				case GLFW_KEY_Q:	  ShiftToNextShader();				return;
				case GLFW_KEY_W:	  SetTimeScale(timeScale + 0.2f);	return;
				case GLFW_KEY_E:	  SetTimeScale(timeScale - 0.2f);	return;
				default: return;
			}
		}
		///<summary>
		///Initialize engine with timescale.
		///Also loads namehashids and sets the initial shader.
		///</summary>
		void Initialize(float timescale)
		{
			SetTimeScale(timescale);

			shaderHashIds = Shader::GetAllNameHashIds();
			currentShaderIndex = 0;

			currentShader = Shader::Find(shaderHashIds.at(currentShaderIndex));
			currentShader->UseProgram();
		}

	private:
		///<summary>
		/// Set current shader timescale and display the new value in the console.
		///</summary>
		void SetTimeScale(float scale)
		{
			timeScale = scale;
			cout << "Shader Timescale is: " << scale << endl;
		}
		///<summary>
		/// Selects the next shader by using a namehashID from shaderHasIds list.
		///</summary>
		void ShiftToNextShader()
		{
			currentShader = Shader::Find(shaderHashIds.at(currentShaderIndex));
			currentShader->UseProgram();
			currentShaderIndex = (currentShaderIndex + 1) % (shaderHashIds.size());
		}
		///<summary>
		/// Calculates new shader time.
		/// To avoid offset in the shadertime value, timescale needs to be applied to deltatime.
		///</summary>
		void CalculateTime()
		{
			float currentTime	= glfwGetTime();
			float deltaTime		= currentTime - previousTime;
			previousTime		= currentTime;
			scaledTime		   += deltaTime * timeScale;
		}

		const int hashRes  = Shader::HashId("_Resolution");
		const int hashTime = Shader::HashId("_Time");

		float previousTime;
		float scaledTime;
		float timeScale;

		Shader*		 currentShader;
		vector<int>  shaderHashIds;
		unsigned int currentShaderIndex;
};

int main(void)
{
	auto config = Config("res/Config.txt");

	::ShowWindow(::GetConsoleWindow(), config.showConsole ? SW_SHOW : SW_HIDE);

	if (!graphics.TryInitialize("GL Context", config.width, config.height))
		exit(EXIT_FAILURE);

	Shader::ImportMultiple("res/Shaders.txt");

	auto shaderEngine = make_shared<ShaderEngine>(); 
	shaderEngine->Initialize(config.timescale);

	graphics.RegisterInputReceiver(shaderEngine);
	graphics.RegisterShaderModifier(shaderEngine);

	while (graphics.TryDraw()) {}

	Exit();
}

/// <summary>
/// Exits application.
/// </summary>
void Exit()
{
	graphics.Terminate();
	Shader::ReleaseAll();
	exit(EXIT_SUCCESS);
}