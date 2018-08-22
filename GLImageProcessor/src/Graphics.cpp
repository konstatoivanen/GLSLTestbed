#include "Graphics.h"
#include "Shader.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include "linmath.h"

unsigned int shader;
int location;
float r;

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

	cout << "OpenGL Context Terminated" << endl;
}

bool Graphics::TryInitialize(const string& title, unsigned int width, unsigned int height)
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(512, 512, "OpenGL Testbed", NULL, NULL);

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

	shader = Shader::LoadShader("res/shaders/Default.shader");

	glUseProgram(shader);

	location = glGetUniformLocation(shader, "_Color");

	cout << "--OpenGL Context Initialized--" << endl;

	return true;
}

bool Graphics::Update()
{
	if (glfwWindowShouldClose(window))
		return false;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);

	r += 0.01f;
	r = fmod(r, 1.0f);

	glUniform4f(location, r, 0.0f, 0.0f, 1.0f);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

void Graphics::OnExit()
{
	glDeleteProgram(shader);
	Terminate();
}
