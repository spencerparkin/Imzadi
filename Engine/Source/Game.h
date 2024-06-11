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

/**
 * This is a melding of engine and game.  The term "engine" is used
 * too generously here.  For that matter, so is the term "game".
 * This class is my poor attempt to use DirectX to make a game.
 */
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

	/**
	 * This doesn't seem like a terribly clever way to manage the controller
	 * input, but it's an attempt to do so, nevertheless.
	 * 
	 * @param[in] controllerUser This is who's asking for the controller interface.
	 * @return A pointer to the controller interface is returned if the given user is allowed to use it; null, otherwise.
	 */
	Controller* GetController(const std::string& controllerUser);

	/**
	 * Make the given user the current user that is allowed to get access
	 * to the controller for input.  Note that if the given user is already
	 * able to get the controller, this is a no-op and the stack is left unchanged.
	 * 
	 * @param[in] controllerUser This is who will become able to get the controller.
	 */
	void PushControllerUser(const std::string& controllerUser);

	/**
	 * The current controller user is removed in favor of whoever is next
	 * in the stack.
	 * 
	 * @return The old top controller user is returned; the empty string on stack-underflow.
	 */
	std::string PopControllerUser();

	/**
	 * Return who can currently use the controller for input.
	 */
	std::string GetControllerUser();
	
	/**
	 * Return the current acceleration due to gravity.  For simplicity,
	 * gravity is always in the -Y direction.
	 */
	double GetGravity() const { return this->accelerationDuetoGravity; }

	/**
	 * Set the current acceleration due to gravity.
	 */
	void SetGravity(double gravity) { this->accelerationDuetoGravity = gravity; }

	void SetCollisionSystemDebugDrawFlags(uint32_t flags) { this->collisionSystemDebugDrawFlags = flags; }
	uint32_t GetCollisionSystemDebugDrawFlags() { return this->collisionSystemDebugDrawFlags; }

private:

	void AdvanceEntities(Entity::TickPass tickPass, double deltaTimeSeconds);
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
	std::vector<std::string> controllerUserStack;
	Collision::System collisionSystem;
	double accelerationDuetoGravity;
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