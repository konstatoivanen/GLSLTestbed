#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

class Graphics
{
	public:
		 Graphics();
		 virtual ~Graphics();
		 bool TryInitialize(const string& title, unsigned int width, unsigned int height);
		 bool Update();
		 void Terminate();
		 void OnExit();

	private:
		GLFWwindow* window;
};

