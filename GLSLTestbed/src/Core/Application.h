#pragma once
#include "PrecompiledHeader.h"
#include "Core/ServiceRegister.h"
#include "Core/Window.h"

int main(int argc, char** argv);

class Application
{
	public:
		Application(const std::string& name = "Application");
		virtual ~Application();
		void Close();
	
		static Application& Get() { return *s_Instance; }
		
		template<typename T>
		static T* GetService() { return Get().m_services->Get<T>(); }

		static const Window& GetWindow() { return *(Get().m_window); }

	private:
		void Run();

	private:
		static Application* s_Instance;
		bool m_Running = true;
		
		Scope<Window> m_window;
		Scope<ServiceRegister> m_services;

		friend int ::main(int argc, char** argv);
};
