#include "PrecompiledHeader.h"
#include "Window.h"
#include "Utilities/Log.h"
#include "GLAD/glad.h"

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
	SetVSync(props.vsync);

	int gladstatus = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	PK_CORE_ASSERT(gladstatus, "Failed To Initialize GLAD");

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
