#pragma once

#if !defined WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <list>
#include <time.h>
#include "Reference.h"
#include "RenderObjects/DebugLines.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"
#include "Controller.h"
#include "Collision/System.h"

#define IMZADI_GAME_WINDOW_CLASS_NAME		TEXT("ImzadiGameWindowClass")

namespace Imzadi
{
	class Scene;
	class AssetCache;
	class Camera;
	class RenderObject;
	class Entity;

	/**
	 * Ticking is performed in three passes per frame.  Collision queries and commands should
	 * be performed in the first tick, if at all or ever.  Work done in parallel with the collision
	 * system should be done in the second tick.  Results of collision queries can be acquired
	 * in the third tick.  No collision queries or commands should be issued in the second or
	 * third tick passes.
	 */
	enum TickPass
	{
		PRE_TICK,
		MID_TICK,
		POST_TICK
	};

	/**
	 * User's of the game engine are expected to create a derivative of this class and
	 * override various methods for their joyous purposes.  For now, only one instance
	 * of this class should ever be created per application since much of the code gets
	 * access to that singleton instance through a static method.
	 */
	class IMZADI_API Game
	{
	public:
		Game(HINSTANCE instance);
		virtual ~Game();

		/**
		 * This is where we initialize the renderer, the collision system, the game-framework, etc.
		 */
		virtual bool Initialize();

		/**
		 * This gets called in Initialize before anything is done.
		 */
		virtual bool PreInit();

		/**
		 * This gets called in Initialize after everything else succeeds.
		 * Initialize entities here.  Note that you can replace the default
		 * AssetCache object with your own derivative in this call as well so
		 * that you can support your own custom entities.
		 */
		virtual bool PostInit();

		/**
		 * If initialization succeeds, the application is expected to call this
		 * repeatedly in a loop until false is returned, afterwhich, the game
		 * should get shutdown.  Overriding this method doesn't provide you any
		 * reasonable way to perform your own work while the engine runs.  The
		 * best way to perform work while the engine ticks is to override the
		 * Tick methods on entities that are added to the game.  See the SpawnEntity method.
		 * You can also override the Tick method of this class.
		 * 
		 * One pattern is to spawn entities in the PostInit() call, then they can
		 * tick while the game runs.  One such entity can represent the entire level.
		 * Once the level entity dies (the level is complete or whatever), then
		 * other entities can be spawned (to return to main-menu or go to the next
		 * level.)  That's the idea, anyway.  The game frame-work is based entirely
		 * around the life-cycle of entities, which can represent any kind of thing
		 * (not just heros and enemies) in the game.
		 * 
		 * Doing any heavy work in the loop that calls this method is not recommended.
		 * An effort has been made to tick entities at certain times during a single
		 * engine tick to try to take advantage of work being done in parallel by other
		 * threads.  This call was not made a blocking call, because the game engine
		 * might be something you want to run in contexts other than a performance-critical
		 * game, such as, for example, a level editor or something like that.  Such
		 * applications have to handle UI button presses and other things enbeknownst to
		 * the game engine.
		 */
		virtual bool Run();

		/**
		 * Override this to do general ticking while the game runs.  Note that most ticking
		 * should probably be done in an entity's tick method.  Be sure to call this base-class
		 * method if you override this it.
		 * 
		 * @param tickPass This let's you know what kind of work is most appropriately done in this pass.
		 */
		virtual void Tick(TickPass tickPass, double deltaTimeSeconds);

		/**
		 * This gets called in Shutdown after everything else is done.
		 */
		virtual bool PreShutdown();

		/**
		 * This gets called in Shutdown before anything else is done.
		 */
		virtual bool PostShutdown();

		/**
		 * This performs the work of shutting down the game-framework, the collisions system,
		 * and the rendering system.  If you override this, don't forget to call this base-class
		 * method as well.
		 */
		virtual bool Shutdown();

		/**
		 * This can be overridden so that the application can provide its own window
		 * instead of letting the game engine create the render window.  You might,
		 * for example, want to embed your render window in an application with other
		 * windows and controls.
		 */
		virtual bool CreateRenderWindow();

		Scene* GetScene() { return this->scene.Get(); }
		AssetCache* GetAssetCache() { return this->assetCache.Get(); }
		Camera* GetCamera() { return this->camera.Get(); }
		void SetCamera(Reference<Camera> camera) { this->camera = camera; }
		CollisionSystem* GetCollisionSystem() { return &this->collisionSystem; }
		DebugLines* GetDebugLines() { return this->debugLines.Get(); }

		static Game* Get();
		static void Set(Game* game);

		ID3D11Device* GetDevice() { return this->device; }
		ID3D11DeviceContext* GetDeviceContext() { return this->deviceContext; }
		HWND GetMainWindowHandle() { return this->mainWindowHandle; }
		ID3D11ShaderResourceView* GetShadowBufferResourceViewForShader() { return this->shadowBufferViewForShader; }
		ID3D11SamplerState* GetShadowBufferSamplerState() { return this->shadowBufferSamplerState; }

		struct LightParams
		{
			Vector3 lightDirection;
			Vector4 lightColor;
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
			const Vector3& position,
			const Quaternion& orientation);

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

	protected:

		void AdvanceEntities(TickPass tickPass, double deltaTimeSeconds);
		
		/**
		 * This is where we make all draw-calls.  I can't think of a good reason to override
		 * this method at this time.  For custom drawing, derive from the RenderObject class.
		 */
		virtual void Render();

		/**
		 * Override this call to handle windows messages, but don't forget to call this
		 * base class method as well.
		 */
		virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK WndProcEntryFunc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);

		bool RecreateViews();

		TCHAR windowTitle[256];
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
		CollisionSystem collisionSystem;
		double accelerationDuetoGravity;
		Reference<DebugLines> debugLines;
		uint32_t collisionSystemDebugDrawFlags;
		clock_t lastTickTime;
		static Game* gameSingleton;
	};

	template<typename T>
	void IMZADI_API SafeRelease(T*& thing)
	{
		if (thing)
		{
			thing->Release();
			thing = nullptr;
		}
	}
}