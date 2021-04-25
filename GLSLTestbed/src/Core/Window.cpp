#include "PrecompiledHeader.h"
#include "Window.h"
#include "Utilities/Log.h"
#include "GLAD/glad.h"

namespace PK::Core
{
	static Window* GetWindowPtr(GLFWwindow* window)
	{
		return (Window*)glfwGetWindowUserPointer(window);
	}
	
	template<typename T, typename ... Args>
	static void SafeInvokeFunction(const T& function, const Args& ... args)
	{
		if (function)
		{
			function(args...);
		}
	}

	static std::string GetGLDebugEnumStr(GLenum glenum)
	{
		switch (glenum)
		{
			case GL_DEBUG_SOURCE_API: return "SOURCE_API";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "SOURCE_WINDOW_SYSTEM";
			case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SOURCE_SHADER_COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY: return "SOURCE_THIRD_PARTY";
			case GL_DEBUG_SOURCE_APPLICATION: return "SOURCE_APPLICATION";
			case GL_DEBUG_SOURCE_OTHER: return "SOURCE_OTHER";
			case GL_DEBUG_TYPE_ERROR: return "TYPE_ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "TYPE_DEPRECATED_BEHAVIOR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "TYPE_UNDEFINED_BEHAVIOR";
			case GL_DEBUG_TYPE_PORTABILITY: return "TYPE_PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE: return "TYPE_PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER: return "TYPE_MARKER";
			case GL_DEBUG_TYPE_PUSH_GROUP: return "TYPE_PUSH_GROUP";
			case GL_DEBUG_TYPE_POP_GROUP: return "TYPE_POP_GROUP";
			case GL_DEBUG_TYPE_OTHER: return "TYPE_OTHER";
			case GL_DEBUG_SEVERITY_HIGH: return "SEVERITY_HIGH";
			case GL_DEBUG_SEVERITY_MEDIUM: return "SEVERITY_MEDIUM";
			case GL_DEBUG_SEVERITY_LOW: return "SEVERITY_LOW";
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "SEVERITY_NOTIFICATION";
		}

		return "Unkown Debug Enum";
	}

	static void OnGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		if (type == GL_DEBUG_TYPE_ERROR)
		{
			PK_CORE_ERROR("GL MSG: %s, %s, %s", GetGLDebugEnumStr(type).c_str(), GetGLDebugEnumStr(severity).c_str(), message);
		}

		PK_CORE_LOG("GL MSG: %s, %s, %s", GetGLDebugEnumStr(type).c_str(), GetGLDebugEnumStr(severity).c_str(), message);
	}
	
	Window::Window(const WindowProperties& props)
	{
		m_properties.title = props.title;
		m_properties.width = props.width;
		m_properties.height = props.height;
	
		PK_CORE_LOG_HEADER("Creating window %s (%i, %i)", props.title.c_str(), props.width, props.height);
	
	#if defined(PK_DEBUG)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	#endif
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		m_window = glfwCreateWindow((int)props.width, (int)props.height, props.title.c_str(), nullptr, nullptr);
	
		if (!m_window)
		{
			PK_CORE_ERROR("Failed To Create Window");
		}
	
		glfwSetWindowUserPointer(m_window, this);
		SetActive();
	
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* nativeWindow, int width, int height)
		{
			auto window = GetWindowPtr(nativeWindow);
			window->m_properties.width = width;
			window->m_properties.height = height;
			window->m_minimized = width == 0 || height == 0;
			SafeInvokeFunction(window->OnResize,width, height);
		});
		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* nativeWindow) 
		{ 	
			auto window = GetWindowPtr(nativeWindow);
			window->m_alive = false;
			SafeInvokeFunction(window->OnClose);
		});
		glfwSetKeyCallback(m_window, [](GLFWwindow* nativeWindow, int key, int scancode, int action, int mods) { SafeInvokeFunction(GetWindowPtr(nativeWindow)->OnKeyInput, key, scancode, action, mods);});
		glfwSetCharCallback(m_window, [](GLFWwindow* nativeWindow, uint32_t keycode) { SafeInvokeFunction(GetWindowPtr(nativeWindow)->OnCharInput, keycode); });
		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* nativeWindow, int button, int action, int mods) { SafeInvokeFunction(GetWindowPtr(nativeWindow)->OnMouseButtonInput, button, action, mods); });
		glfwSetScrollCallback(m_window, [](GLFWwindow* nativeWindow, double xOffset, double yOffset) { SafeInvokeFunction(GetWindowPtr(nativeWindow)->OnScrollInput, xOffset, yOffset); });
		glfwSetCursorPosCallback(m_window, [](GLFWwindow* nativeWindow, double xPos, double yPos) {SafeInvokeFunction(GetWindowPtr(nativeWindow)->OnCursorInput, xPos, yPos); });
		glfwSetErrorCallback([](int error, const char* description) {PK_CORE_ERROR("GLFW Error (%i) : %s", error, description); });
		SetVSync(props.vsync);

		// @TODO fix this being called per window
		int gladstatus = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		PK_CORE_ASSERT(gladstatus, "Failed To Initialize GLAD");

		glFrontFace(GL_CW);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		#if defined(PK_DEBUG)
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(OnGLMessageCallback, 0);
		#endif

		m_alive = true;
	}
	
	Window::~Window()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);
			m_window = nullptr;
		}
	}
	
	void Window::SetVSync(bool enabled)
	{
		glfwSwapInterval(enabled ? 1 : 0);
		m_properties.vsync = enabled;
	}
	
	void Window::SetActive() 
	{ 
		glfwMakeContextCurrent(m_window); 
	}
}