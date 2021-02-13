#include "PrecompiledHeader.h"
#include "Core/Application.h"

int main(int argc, char** argv)
{
	auto app = new Application("GLSL Testbed");
	app->Run();
	delete app;
}