#include "Main.h"
#include "Game.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
	Game game(instance);

	if (game.Initialize())
	{
		game.Run();
	}

	game.Shutdown();

	return 0;
}