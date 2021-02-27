#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/StringHashID.h"
#include "Rendering/Graphics.h"
#include "Rendering/Objects/Shader.h"
#include "Core/ServiceRegister.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/ApplicationConfig.h"
#include "Core/Window.h"
#include "Core/Sequencer.h"

int main(int argc, char** argv);

class Application
{
	public:
		Application(const std::string& name = "Application");
		virtual ~Application();
		void Close();
	
		static Application& Get() { return *s_Instance; }
		
		template<typename T>
		static Ref<T> GetService() { return Get().m_services->GetService<T>(); }

		static const Window& GetWindow() { return *(Get().m_window); }

	private:
		void Run();

	private:
		static Application* s_Instance;
		bool m_Running = true;

		Scope<Window> m_window;
		Scope<ServiceRegister> m_services;
		PKECS::Sequencer m_sequencer;
		StringHashID::Cache m_hashCache;
		GraphicsContext m_graphicsContext;

		friend int ::main(int argc, char** argv);
};
