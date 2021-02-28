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
	auto app = new Application("GLSL Testbed");
	app->Run();
	delete app;

#ifdef PK_DEBUG_LEAKS
	_CrtDumpMemoryLeaks();
#endif
}