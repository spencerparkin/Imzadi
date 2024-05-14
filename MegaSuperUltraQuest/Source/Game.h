#pragma once

#include <Windows.h>
#include <d3d11.h>

#define GAME_WINDOW_CLASS_NAME		TEXT("GameWindowClass")

class Game
{
public:
	Game(HINSTANCE instance);
	virtual ~Game();

	bool Initialize();
	bool Run();
	bool Shutdown();

private:

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProcEntryFunc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);

	HINSTANCE instance;
	HWND mainWindowHandle;
	bool keepRunning;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* frameBufferView;
};