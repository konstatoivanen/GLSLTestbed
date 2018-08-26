#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Dispatcher.h"

using namespace std;

class Graphics
{
	public:
		 Graphics();
		 virtual ~Graphics();
		 /// <summary>
		 /// Tries to initialize a new graphics context.
		 /// </summary>
		 bool TryInitialize(const string& title, unsigned int width, unsigned int height);
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
		 /// <summary>
		 /// Releases All loaded shaders.
		 /// Call this when exiting the application.
		 /// </summary>
		 void OnExit();

		 Dispatcher<void(const int& width, const int& height)> beforeDrawCallBacks;
		 GLFWwindow* window;
};

