#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include "Reference.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"

#define GAME_WINDOW_CLASS_NAME		TEXT("GameWindowClass")

class Scene;
class AssetCache;
class Camera;
class RenderObject;

class Game
{
public:
	Game(HINSTANCE instance);
	virtual ~Game();

	bool Initialize();
	bool Run();
	bool Shutdown();

	Scene* GetScene() { return this->scene.Get(); }
	AssetCache* GetAssetCache() { return this->assetCache.Get(); }
	Camera* GetCamera() { return this->camera.Get(); }
	void SetCamera(Reference<Camera> camera) { this->camera = camera; }

	static Game* Get() { return gameSingleton; }
	static void Set(Game* game) { gameSingleton = game; }

	ID3D11Device* GetDevice() { return this->device; }
	ID3D11DeviceContext* GetDeviceContext() { return this->deviceContext; }
	HWND GetMainWindowHandle() { return this->mainWindowHandle; }

private:

	void Render();
	Reference<RenderObject> LoadAndPlaceRenderMesh(
			const std::string& renderMeshFile,
			const Collision::Vector3& position,
			const Collision::Quaternion& orientation,
			const Collision::Vector4& color);

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProcEntryFunc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);

	HINSTANCE instance;
	HWND mainWindowHandle;
	bool keepRunning;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* frameBufferView;
	ID3D11RasterizerState* rasterizerState;
	Reference<Scene> scene;
	Reference<AssetCache> assetCache;
	Reference<Camera> camera;
	static Game* gameSingleton;
};