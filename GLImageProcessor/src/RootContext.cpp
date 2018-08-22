#include <Windows.h>
#include "Graphics.h"

using namespace std;

int main(void)
{
	//::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	Graphics graphics = Graphics();

	if (!graphics.TryInitialize("GL Context", 512, 512))
		exit(EXIT_FAILURE);

	while (graphics.Update())
	{
	}

	graphics.OnExit();

	exit(EXIT_SUCCESS);
}