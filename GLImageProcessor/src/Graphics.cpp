#include <Windows.h>
#include "Graphics.h"
#include "Shader.h"

/// <summary>
/// Initializes the vertex streams required for drawing a fullscreen quad
/// </summary>
static void InitializeFullScreenQuad()
{
	float vertices[] =
	{
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f,
		1.0f, -1.0f,
	};

	unsigned int indices[] =
	{
		0,1,2,
		2,3,0
	};

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * sizeof(float), indices, GL_STATIC_DRAW);
}
static void GLErrorCallback(int, const char* err_str)
{
	std::cout << "GLFW ERROR! : " << err_str << endl;
}

Graphics::Graphics()
{

}
Graphics::~Graphics()
{

}

void Graphics::Terminate()
{
	if(window) glfwDestroyWindow(window);
	glfwTerminate();

	beforeDrawCallBacks.clear();

	cout << "OpenGL Context Terminated" << endl;
}
bool Graphics::TryInitialize(const string& title, unsigned int width, unsigned int height)
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "OpenGL Testbed", NULL, NULL);

	if (!window)
	{
		Terminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		Terminate();
		return false;
	}

	glfwSetErrorCallback(GLErrorCallback);

	InitializeFullScreenQuad();

	cout << "--OpenGL Context Initialized--" << endl;

	return true;
}
bool Graphics::TryDraw()
{
	if (glfwWindowShouldClose(window))
		return false;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);

	beforeDrawCallBacks(width, height);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	glfwSwapBuffers(window);
	return true;
}
void Graphics::OnExit()
{
	Shader::ReleaseAll();
	Terminate();
}