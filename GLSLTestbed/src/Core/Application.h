#pragma once
#include "PrecompiledHeader.h"
#include "Rendering/Graphics.h"
#include "Rendering/Objects/Shader.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/ApplicationConfig.h"
#include "Core/Window.h"

int main(int argc, char** argv);

class Application
{
	public:
		Application(const std::string& name = "Application");
		virtual ~Application();
		void Close();
	
		static Application& Get() { return *s_Instance; }
		static Time& GetTime() { return Get().m_time; }
		static AssetDatabase& GetAssetDatabase() { return Get().m_assetDatabase; }
		static ApplicationConfig& GetConfig() { return Get().m_applicationConfig; }
		static Input& GetInput() { return Get().m_input; }
		static const Window& GetWindow() { return *(Get().m_window); }

	private:
		void Run();

	private:
		static Application* s_Instance;
		bool m_Running = true;

		Scope<Window> m_window;
		GraphicsContext m_graphicsContext;
		ApplicationConfig m_applicationConfig;
		AssetDatabase m_assetDatabase;
		Input m_input;
		Time m_time;

		friend int ::main(int argc, char** argv);
};
