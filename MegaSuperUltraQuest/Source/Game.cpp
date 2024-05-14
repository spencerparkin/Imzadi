#include "Game.h"

Game::Game(HINSTANCE instance)
{
	this->instance = instance;
	this->mainWindowHandle = NULL;
	this->keepRunning = false;
}

/*virtual*/ Game::~Game()
{
}

bool Game::Initialize()
{
	WNDCLASSEX winClass;
	::ZeroMemory(&winClass, sizeof(winClass));
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = &Game::WndProcEntryFunc;
	winClass.lpszClassName = GAME_WINDOW_CLASS_NAME;
	
	if (!RegisterClassEx(&winClass))
		return false;

	RECT rect = { 0, 0, 1024, 768 };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	this->mainWindowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
												winClass.lpszClassName,
												TEXT("Mega Super Ultra Quest"),
												WS_OVERLAPPEDWINDOW | WS_VISIBLE,
												CW_USEDEFAULT, CW_USEDEFAULT,
												width,
												height,
												0, 0, this->instance, 0);

	if (this->mainWindowHandle == NULL)
	{
		return false;
	}

	SetWindowLongPtr(this->mainWindowHandle, GWLP_USERDATA, LONG_PTR(this));

	this->keepRunning = true;
	return true;
}

bool Game::Run()
{
	while (this->keepRunning)
	{
		MSG message{};
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&message);
		}

		// TODO: Game tick/render goes here.
	}

	return true;
}

LRESULT Game::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CLOSE:
		case WM_QUIT:
		{
			this->keepRunning = false;
			break;
		}
	}

	return ::DefWindowProc(this->mainWindowHandle, msg, wParam, lParam);
}

/*static*/ LRESULT Game::WndProcEntryFunc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR userData = GetWindowLongPtr(windowHandle, GWLP_USERDATA);
	if (userData != 0)
	{
		auto game = (Game*)userData;
		return game->WndProc(msg, wParam, lParam);
	}

	return DefWindowProc(windowHandle, msg, wParam, lParam);
}

bool Game::Shutdown()
{
	UnregisterClass(GAME_WINDOW_CLASS_NAME, this->instance);

	return true;
}