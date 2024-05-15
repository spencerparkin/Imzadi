#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <memory>

#define GAME_WINDOW_CLASS_NAME		TEXT("GameWindowClass")

class Scene;
class AssetCache;

class Game
{
public:
	Game(HINSTANCE instance);
	virtual ~Game();

	bool Initialize();
	bool Run();
	bool Shutdown();

	Scene* GetScene() { return this->scene.get(); }
	AssetCache* GetAssetCache() { return this->assetCache.get(); }

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
	std::shared_ptr<Scene> scene;
	std::shared_ptr<AssetCache> assetCache;
};