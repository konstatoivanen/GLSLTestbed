#pragma once
#include "PrecompiledHeader.h"
#include "Core/ServiceRegister.h"
#include "Core/Window.h"
#include "Core/NoCopy.h"

int main(int argc, char** argv);

namespace PK::Core
{
	using namespace Utilities;

	class Application : public NoCopy
	{
		public:
			Application(const std::string& name = "Application");
			virtual ~Application();
			void Close();
		
			inline static Application& Get() { return *s_Instance; }
			
			template<typename T>
			inline static T* GetService() { return Get().m_services->Get<T>(); }
	
			inline static Window* GetWindow() { return Get().m_window.get(); }
	
		private:
			void Run();
	
		private:
			static Application* s_Instance;
			bool m_Running = true;
			
			Scope<Window> m_window;
			Scope<ServiceRegister> m_services;
	
			friend int ::main(int argc, char** argv);
	};
}