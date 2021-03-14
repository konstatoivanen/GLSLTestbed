#include "PrecompiledHeader.h"

#define PK_DEBUG_LEAKS

#ifdef PK_DEBUG_LEAKS
	#define _CRTDBG_MAP_ALLOC  
	#include <stdlib.h>  
	#include <crtdbg.h>  
#endif 

#include "Core/Application.h"

int main(int argc, char** argv)
{
#ifdef PK_DEBUG_LEAKS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	auto app = new Application("GLSL Testbed");
	app->Run();
	delete app;
}