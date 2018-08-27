#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class ShaderModifier
{
	public: virtual void OnDraw(int width, int height) = 0;
			virtual ~ShaderModifier() {};
};

class InputReceiver
{
	public: virtual void OnInput(int key, int scancode, int action, int mods) = 0;
			virtual ~InputReceiver() {};
};

class Graphics
{
	public:
		 /// <summary>
		 /// Tries to initialize a new graphics context.
		 /// </summary>
		 bool TryInitialize(const std::string& title, unsigned int width, unsigned int height);
		 /// <summary>
		 /// Draws a new frame.
		 /// Fails if application try close is detected.
		 /// Triggers onBlitCallbacks.
		 /// </summary>
		 bool TryDraw();
		 /// <summary>
		 /// Terminates current graphics context.
		 /// Call this if you want to reinitialize graphics or exit.
		 /// </summary>
		 void Terminate();
		 ///<summary>
		 /// Register ShaderModifier instance for updates.
		 /// Updates are called just before rendering.
		 ///</summary>
		 void RegisterShaderModifier(const std::shared_ptr<ShaderModifier>& shaderModifier);
		 ///<summary>
		 /// Register InputReceiver instance for glfw input events
		 ///</summary>
		 void RegisterInputReceiver(const std::shared_ptr<InputReceiver>& inputReceiver);

	private:
		 static void GLInputCallback(GLFWwindow*, int key, int scancode, int action, int mods);

		 GLFWwindow* window;
		 std::vector<std::shared_ptr<ShaderModifier>>		shaderModifiers;
		 static std::vector<std::shared_ptr<InputReceiver>> inputReceivers;
};
#endif