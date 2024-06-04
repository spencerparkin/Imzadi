#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <list>
#include <time.h>
#include "Reference.h"
#include "RenderObjects/DebugLines.h"
#include "Entity.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"
#include "Controller.h"
#include "System.h"

#define GAME_WINDOW_CLASS_NAME		TEXT("GameWindowClass")

class Scene;
class AssetCache;
class Camera;
class RenderObject;
class Entity;

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
	Collision::System* GetCollisionSystem() { return &this->collisionSystem; }
	DebugLines* GetDebugLines() { return this->debugLines.Get(); }

	static Game* Get() { return gameSingleton; }
	static void Set(Game* game) { gameSingleton = game; }

	ID3D11Device* GetDevice() { return this->device; }
	ID3D11DeviceContext* GetDeviceContext() { return this->deviceContext; }
	HWND GetMainWindowHandle() { return this->mainWindowHandle; }
	ID3D11ShaderResourceView* GetShadowBufferResourceViewForShader() { return this->shadowBufferViewForShader; }
	ID3D11SamplerState* GetShadowBufferSamplerState() { return this->shadowBufferSamplerState; }

	struct LightParams
	{
		Collision::Vector3 lightDirection;
		Collision::Vector4 lightColor;
		double directionalLightIntensity;
		double ambientLightIntensity;
		double lightCameraDistance;
	};

	const LightParams& GetLightParams() const { return this->lightParams; }
	LightParams& GetLightParams() { return this->lightParams; }
	Camera* GetLightSourceCamera() { return this->lightSourceCamera.Get(); }

	template<typename T>
	T* SpawnEntity()
	{
		T* entity = new T();
		this->spawnedEntityQueue.push_back(entity);
		return entity;
	}

	Reference<RenderObject> LoadAndPlaceRenderMesh(
								const std::string& renderMeshFile,
								const Collision::Vector3& position,
								const Collision::Quaternion& orientation);

	Controller* GetController() { return &this->controller; }

	void SetCollisionSystemDebugDrawFlags(uint32_t flags) { this->collisionSystemDebugDrawFlags = flags; }
	uint32_t GetCollisionSystemDebugDrawFlags() { return this->collisionSystemDebugDrawFlags; }

private:

	void AdvanceEntities(double deltaTimeSeconds);
	void Render();

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProcEntryFunc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);

	bool RecreateViews();

	HINSTANCE instance;
	HWND mainWindowHandle;
	bool keepRunning;
	bool windowResized;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* frameBufferView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11RasterizerState* mainPassRasterizerState;
	ID3D11RasterizerState* shadowPassRasterizerState;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11DepthStencilView* shadowBufferView;
	ID3D11ShaderResourceView* shadowBufferViewForShader;
	ID3D11SamplerState* shadowBufferSamplerState;
	D3D11_VIEWPORT mainPassViewport;
	D3D11_VIEWPORT shadowPassViewport;
	Reference<Scene> scene;
	Reference<AssetCache> assetCache;
	Reference<Camera> camera;
	Reference<Camera> lightSourceCamera;
	LightParams lightParams;
	std::list<Reference<Entity>> spawnedEntityQueue;
	std::list<Reference<Entity>> tickingEntityList;
	Controller controller;
	Collision::System collisionSystem;
	Reference<DebugLines> debugLines;
	uint32_t collisionSystemDebugDrawFlags;
	clock_t lastTickTime;
	static Game* gameSingleton;
};

template<typename T>
void SafeRelease(T*& thing)
{
	if (thing)
	{
		thing->Release();
		thing = nullptr;
	}
}