#include "Main.h"
#include "GameApp.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
	GameApp game(instance);
	GameApp::Set(&game);

	if (game.Initialize())
	{
		while (game.Run())
		{
		}
	}

	game.Shutdown();

	return 0;
}