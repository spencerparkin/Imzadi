#include "Main.h"
#include "Game.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
	Game game(instance);

	if (!game.Initialize())
	{
		//MessageBoxA()
		return -1;
	}

	game.Run();

	game.Shutdown();

	return 0;
}