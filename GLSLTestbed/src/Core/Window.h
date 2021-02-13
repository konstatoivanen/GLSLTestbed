#pragma once
#include "PrecompiledHeader.h"
#include "Rendering/Graphics.h"

struct WindowProperties
{
	std::string title;
	uint32_t width;
	uint32_t height;
	bool vsync;

	WindowProperties(const std::string& title = "OpenGL Window", uint32_t width = 1600, uint32_t height = 900, bool vsync = true) : title(title), width(width), height(height), vsync(vsync)
	{
	}
};

class Window
{
	public:
		Window(const WindowProperties& properties);
		~Window();
	
		uint32_t GetWidth() const { return m_properties.width; }
		uint32_t GetHeight() const { return m_properties.height; }
		float GetAspect() const { return (float)m_properties.width / m_properties.height; }
		bool IsAlive() const { return m_alive; }
		bool IsMinimized() const { return m_minimized; }
		bool IsVSync() const { return m_properties.vsync; }
		void SetVSync(bool enabled);
		GLFWwindow* GetNativeWindow() const { return m_window; }
	
	public:
		std::function<void(int width, int height)> OnResize;
		std::function<void()> OnClose;
		std::function<void(int key, int scancode, int action, int mods)> OnKeyInput;
		std::function<void(unsigned int keycode)> OnCharInput;
		std::function<void(int button, int action, int mods)> OnMouseButtonInput;
		std::function<void(double xOffset, double yOffset)> OnScrollInput;
		std::function<void(double xPos, double yPos)> OnCursorInput;
	
	private:
		GLFWwindow* m_window;
		WindowProperties m_properties;
		bool m_alive = true;
		bool m_minimized = false;
};