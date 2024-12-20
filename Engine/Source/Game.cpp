#include "Game.h"
#include "Scene.h"
#include "Assets/RenderMesh.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "RenderObjects/TextRenderObject.h"
#include "RenderObjects/CollisionStatsRenderObject.h"
#include "RenderObjects/ConsoleRenderObject.h"
#include "Camera.h"
#include "Entities/Level.h"
#include "Math/Transform.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Log.h"
#include <format>
#include <math.h>

using namespace Imzadi;

Game* Game::gameSingleton = nullptr;

Game::Game(HINSTANCE instance)
{
	this->accelerationDuetoGravity = 40.0;
	this->collisionSystemDebugDrawFlags = 0;
	this->debugDrawVisibilityBoxes = false;
	this->deltaTimeSeconds = 0.0;
	this->instance = instance;
	this->mainWindowHandle = NULL;
	this->keepRunning = false;
	this->windowResized = false;
	this->device = NULL;
	this->deviceContext = NULL;
	this->swapChain = NULL;
	this->frameBufferView = NULL;
	this->depthStencilView = NULL;
	this->shadowBufferView = NULL;
	this->shadowBufferViewForShader = NULL;
	this->generalSamplerState = NULL;
	this->scene = nullptr;
	this->assetCache = nullptr;
	this->lightParams.lightDirection = Vector3(0.2, -1.0, 0.2).Normalized();
	this->lightParams.lightColor.SetComponents(1.0, 1.0, 1.0, 1.0);
	this->lightParams.directionalLightIntensity = 1.0;
	this->lightParams.ambientLightIntensity = 0.1;
	this->lightParams.lightCameraDistance = 200.0;
	ZeroMemory(&this->mainPassViewport, sizeof(D3D11_VIEWPORT));
	ZeroMemory(&this->shadowPassViewport, sizeof(D3D11_VIEWPORT));
	lstrcpy(this->windowTitle, "Imzadi Game Engine");
}

/*virtual*/ Game::~Game()
{
}

/*virtual*/ HICON Game::GetWindowIcon()
{
	return NULL;
}

/*virtual*/ bool Game::CreateRenderWindow()
{
	WNDCLASSEX winClass;
	::ZeroMemory(&winClass, sizeof(winClass));
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = &Game::WndProcEntryFunc;
	winClass.lpszClassName = IMZADI_GAME_WINDOW_CLASS_NAME;
	winClass.hIcon = this->GetWindowIcon();

	if (!RegisterClassEx(&winClass))
	{
		IMZADI_LOG_ERROR("Failed to register window class!  Error code: %d", GetLastError());
		return false;
	}

	RECT rect = { 0, 0, 1700, 800 };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	this->mainWindowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
												winClass.lpszClassName,
												this->windowTitle,
												WS_OVERLAPPEDWINDOW | WS_VISIBLE,
												CW_USEDEFAULT, CW_USEDEFAULT,
												width,
												height,
												0, 0, this->instance, 0);

	if (this->mainWindowHandle == NULL)
	{
		IMZADI_LOG_ERROR("Failed to create main window!  Error code: %d", GetLastError());
		return false;
	}

	SetWindowLongPtr(this->mainWindowHandle, GWLP_USERDATA, LONG_PTR(this));

	return true;
}

/*virtual*/ bool Game::PreInit()
{
	return true;
}

/*virtual*/ bool Game::PostInit()
{
	if (!this->assetCache)
		this->assetCache.Set(new AssetCache());

	this->assetCache->AddAssetFolder(R"(Engine\Assets)");

	this->scene.Set(new Scene());
	this->debugLines.Set(new DebugLines());
	this->scene->AddRenderObject(this->debugLines.Get());

	return true;
}

/*static*/ Game* Game::Get()
{
	return gameSingleton;
}

/*static*/ void Game::Set(Game* game)
{
	gameSingleton = game;
}

void Game::SetMainWindowHandle(HWND windowHandle)
{
	this->mainWindowHandle = windowHandle;
}

HWND Game::GetMainWindowHandle()
{
	return this->mainWindowHandle;
}

Scene* Game::GetScene()
{
	return this->scene.Get();
}

Camera* Game::GetCamera()
{
	return this->camera.Get();
}

void Game::SetCamera(Reference<Camera> camera)
{
	this->camera = camera;
}

Collision::System* Game::GetCollisionSystem()
{
	return &this->collisionSystem;
}

AudioSystem* Game::GetAudioSystem()
{
	return &this->audioSystem;
}

EventSystem* Game::GetEventSystem()
{
	return &this->eventSystem;
}

DebugLines* Game::GetDebugLines()
{
	return this->debugLines.Get();
}

/*virtual*/ bool Game::Initialize()
{
	IMZADI_LOG_INFO("Initializing game...");

	if (!this->PreInit())
	{
		IMZADI_LOG_ERROR("Pre-initialization failed.");
		return false;
	}

	if (!this->CreateRenderWindow())
	{
		IMZADI_LOG_ERROR("Failed to create (or acquire) render window.");
		return false;
	}

	if (!this->inputSystem.Setup(this->mainWindowHandle))
	{
		IMZADI_LOG_ERROR("Failed to initialize input system.");
		MessageBox(NULL, TEXT("Failed to initialize input system!"), TEXT("Error!"), MB_ICONERROR | MB_OK);
		return false;
	}

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = this->mainWindowHandle;

	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
													creationFlags, featureLevels, ARRAYSIZE(featureLevels),
													D3D11_SDK_VERSION, &swapChainDesc, &this->swapChain,
													&this->device, NULL, &this->deviceContext);

	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create D3D11 device, context and swap-chain!  Error code: %d", GetLastError());
		return false;
	}

#if defined _DEBUG
	ID3D11Debug* debug = nullptr;
	device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
	if (debug)
	{
		ID3D11InfoQueue* infoQueue = nullptr;
		result = debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue);
		if (SUCCEEDED(result))
		{
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->Release();
		}
		debug->Release();
	}
#endif

	this->SetCamera(new Camera());
	this->camera->SetViewMode(Camera::ViewMode::PERSPECTIVE);
	if (!this->camera->LookAt(Vector3(-20.0, 30.0, 50.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)))
		return false;

	this->windowResized = false;
	if (!this->RecreateViews())
	{
		IMZADI_LOG_ERROR("Initial view creation failed.");
		return false;
	}

	Camera::OrthographicParams orthoParams{};
	orthoParams.nearClip = 0.0;
	orthoParams.farClip = 10000.0;
	orthoParams.width = 400.0;
	orthoParams.height = 400.0;

	this->lightSourceCamera.Set(new Camera());
	this->lightSourceCamera->SetViewMode(Camera::ViewMode::ORTHOGRAPHIC);
	this->lightSourceCamera->SetOrthographicParams(orthoParams);

	this->shadowPassViewport.Width = 4096.0f;
	this->shadowPassViewport.Height = 4096.0f;
	this->shadowPassViewport.TopLeftX = 0.0f;
	this->shadowPassViewport.TopLeftY = 0.0f;
	this->shadowPassViewport.MinDepth = 0.0f;
	this->shadowPassViewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC shadowBufferDesc{};
	shadowBufferDesc.Width = 4096;
	shadowBufferDesc.Height = 4096;
	shadowBufferDesc.MipLevels = 1;
	shadowBufferDesc.ArraySize = 1;
	shadowBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowBufferDesc.SampleDesc.Count = 1;
	shadowBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* shadowBuffer = nullptr;
	result = this->device->CreateTexture2D(&shadowBufferDesc, NULL, &shadowBuffer);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create shadow buffer.  Error code: %d", result);
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowViewDesc{};
	shadowViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowViewDesc.Flags = 0;

	result = this->device->CreateDepthStencilView(shadowBuffer, &shadowViewDesc, &this->shadowBufferView);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create depth stencil view for shadow buffer.  Error code: %d", result);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowShaderViewDesc{};
	shadowShaderViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowShaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowShaderViewDesc.Texture2D.MipLevels = 1;

	result = this->device->CreateShaderResourceView(shadowBuffer, &shadowShaderViewDesc, &this->shadowBufferViewForShader);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create shader resource view for shadow buffer.  Error code: %d", result);
		return false;
	}

	shadowBuffer->Release();
	shadowBuffer = nullptr;

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	result = Game::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &this->generalSamplerState);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("CreateSamplerState() failed with error code: {}", result));
		return false;
	}

	if (!this->audioSystem.Initialize())
	{
		IMZADI_LOG_ERROR("Failed to initialize the audio sub-system.");
		return false;
	}

	if (!this->PostInit())
	{
		IMZADI_LOG_ERROR("Post initialization failed.");
		return false;
	}

	this->keepRunning = true;
	return true;
}

Reference<RenderObject> Game::LoadAndPlaceRenderMesh(const std::string& renderMeshFile, const Transform* objectToWorld /*= nullptr*/)
{
	Reference<RenderObject> renderMesh;
	Reference<Asset> renderMeshAsset;

	if (this->assetCache->LoadAsset(renderMeshFile, renderMeshAsset))
	{
		if (renderMeshAsset->MakeRenderInstance(renderMesh))
		{
			if (objectToWorld)
			{
				auto instance = dynamic_cast<RenderMeshInstance*>(renderMesh.Get());
				if (instance)
					instance->SetObjectToWorldTransform(*objectToWorld);
			}

			this->scene->AddRenderObject(renderMesh);
		}
	}

	return renderMesh;
}

bool Game::RecreateViews()
{
	SafeRelease(this->frameBufferView);
	SafeRelease(this->depthStencilView);

	HRESULT result = 0;

	if (this->windowResized)
	{
		result = this->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to resize swap-chain according to new window size.  Error code: %d", result);
			return false;
		}
	}

	ID3D11Texture2D* backBufferTexture = nullptr;
	result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to get back-buffer from swap-chain.  Error code: %d", result);
		return false;
	}

	result = this->device->CreateRenderTargetView(backBufferTexture, NULL, &this->frameBufferView);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create render target view of back buffer.  Error code: %d", result);
		return false;
	}

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	backBufferTexture->GetDesc(&depthBufferDesc);

	backBufferTexture->Release();
	backBufferTexture = nullptr;

	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthBuffer = nullptr;
	result = this->device->CreateTexture2D(&depthBufferDesc, NULL, &depthBuffer);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create depth buffer.  Error code: %d", result);
		return false;
	}

	result = this->device->CreateDepthStencilView(depthBuffer, NULL, &this->depthStencilView);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to depth stencil view of depth buffer.  Error code: %d", result);
		return false;
	}

	depthBuffer->Release();
	depthBuffer = nullptr;

	RECT clientRect;
	GetClientRect(this->mainWindowHandle, &clientRect);

	this->mainPassViewport.TopLeftX = 0.0f;
	this->mainPassViewport.TopLeftY = 0.0f;
	this->mainPassViewport.Width = FLOAT(clientRect.right - clientRect.left);
	this->mainPassViewport.Height = FLOAT(clientRect.bottom - clientRect.top);
	this->mainPassViewport.MinDepth = 0.0f;
	this->mainPassViewport.MaxDepth = 1.0f;

	IMZADI_LOG_INFO("Viewport dimensions: %d x %d", int(this->mainPassViewport.Width), int(this->mainPassViewport.Height));

	double aspectRatio = double(this->mainPassViewport.Width) / double(this->mainPassViewport.Height);

	Frustum frustum;
	frustum.SetFromAspectRatio(aspectRatio, M_PI / 3.0, 0.1, 10000.0);
	this->camera->SetFrustum(frustum);
	Camera::OrthographicParams orthoParams = this->camera->GetOrthographicParameters();
	orthoParams.desiredAspectRatio = aspectRatio;
	this->camera->SetOrthographicParams(orthoParams);

	return true;
}

/*virtual*/ void Game::PumpWindowsMessages()
{
	MSG message{};
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
			this->keepRunning = false;

		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

/*virtual*/ bool Game::Run()
{
	if (this->frameClock.NeverBeenReset())
		this->frameClock.Reset();
	this->deltaTimeSeconds = this->frameClock.GetCurrentTimeSeconds();
	this->frameClock.Reset();
	if (this->deltaTimeSeconds >= 0.1)	// This is useful for being paused in the debugger.
		return true;

	this->debugLines->Clear();

	this->inputSystem.Tick(this->deltaTimeSeconds);
	this->audioSystem.Tick(this->deltaTimeSeconds);

	this->PumpWindowsMessages();

	this->CreateOrDestroyEntities();

	this->Tick(TickPass::MOVE_UNCONSTRAINTED);
	this->collisionSystem.FlushAllTasks();
	this->Tick(TickPass::SUBMIT_COLLISION_QUERIES);
	this->Tick(TickPass::PARALLEL_WORK);
	this->collisionSystem.FlushAllTasks();
	this->Tick(TickPass::RESOLVE_COLLISIONS);

	if (this->windowResized)
	{
		this->deviceContext->OMSetRenderTargets(0, NULL, NULL);
		this->RecreateViews();
		this->windowResized = false;
	}

	if (this->collisionSystemDebugDrawFlags != 0)
	{
		auto query = new Collision::DebugRenderQuery();
		query->SetDrawFlags(this->collisionSystemDebugDrawFlags);
		Collision::TaskID collisionSystemDebugDrawTaskID = 0;
		this->collisionSystem.MakeQuery(query, collisionSystemDebugDrawTaskID);
		this->collisionSystem.FlushAllTasks();
		Collision::Result* result = this->collisionSystem.ObtainQueryResult(collisionSystemDebugDrawTaskID);
		if (result)
		{
			auto debugRenderResult = dynamic_cast<Collision::DebugRenderResult*>(result);
			if (debugRenderResult)
				for (const Collision::DebugRenderResult::RenderLine& line : debugRenderResult->GetRenderLineArray())
					this->debugLines->AddLine({ line.color, line.line });

			delete result;
		}
	}

	if (this->debugDrawVisibilityBoxes)
		this->scene->DrawVisibilityBoxes();

	this->Render();

	return this->keepRunning;
}

void Game::NotifyWindowResized()
{
	this->windowResized = true;
}

AssetCache* Game::GetAssetCache()
{
	return this->assetCache.Get();
}

void Game::SetAssetCache(AssetCache* assetCache)
{
	this->assetCache.Set(assetCache);
}

void Game::AddEntity(Entity* entity)
{
	this->spawnedEntityQueue.push_back(entity);
}

bool Game::FindEntityByName(const std::string& name, Reference<Entity>& foundEntity)
{
	for (auto& entity : this->tickingEntityList)
	{
		if (entity->GetName() == name)
		{
			foundEntity = entity;
			return true;
		}
	}

	for (auto& entity : this->spawnedEntityQueue)
	{
		if (entity->GetName() == name)
		{
			foundEntity = entity;
			return true;
		}
	}

	return false;
}

bool Game::FindAllEntitiesWithName(const std::string& name, std::vector<Entity*>& foundEntityArray)
{
	foundEntityArray.clear();

	for (auto& entity : this->tickingEntityList)
		if (entity->GetName() == name)
			foundEntityArray.push_back(entity);

	return foundEntityArray.size() > 0;
}

bool Game::FindEntityByShapeID(Collision::ShapeID shapeID, Reference<Entity>& foundEntity)
{
	for (auto& entity : this->tickingEntityList)
	{
		if (entity->OwnsCollisionShape(shapeID))
		{
			foundEntity = entity;
			return true;
		}
	}

	return false;
}

void Game::CreateOrDestroyEntities()
{
	bool resortNeeded = false;

	while (this->spawnedEntityQueue.size() > 0)
	{
		std::list<Reference<Entity>>::iterator iter = this->spawnedEntityQueue.begin();
		Reference<Entity> entity = *iter;
		this->spawnedEntityQueue.erase(iter);
		if (!entity->Setup())
			entity->Shutdown();
		else
		{
			this->tickingEntityList.push_back(entity);
			resortNeeded = true;
		}
	}

	auto iter = this->tickingEntityList.begin();
	while (iter != this->tickingEntityList.end())
	{
		auto nextIter = iter;
		nextIter++;

		Entity* entity = *iter;
		if (entity->IsDoomed())
		{
			entity->Shutdown();
			this->tickingEntityList.erase(iter);
		}

		iter = nextIter;
	}

	if (resortNeeded)
	{
		this->tickingEntityList.sort([](const Entity* entityA, const Entity* entityB) -> bool
			{
				return entityA->TickOrder() < entityB->TickOrder();
			});
	}
}

void Game::AdvanceEntities(TickPass tickPass)
{
	for (Entity* entity : this->tickingEntityList)
	{
		if (!entity->Tick(tickPass, this->deltaTimeSeconds))
		{
			entity->DoomEntity();
		}
	}
}

/*virtual*/ void Game::Render()
{
	this->scene->PreRender();

	Vector3 lightCameraPosition = this->camera->GetEyePoint() - this->lightParams.lightCameraDistance * this->lightParams.lightDirection;
	if (!this->lightSourceCamera->LookAt(lightCameraPosition, this->camera->GetEyePoint(), Vector3(0.0, 1.0, 0.0)))
	{
		Transform lightCameraToWorld;
		lightCameraToWorld.translation = lightCameraPosition;
		lightCameraToWorld.matrix.SetColumnVectors(Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, -1.0), Vector3(0.0, 1.0, 0.0));
		this->lightSourceCamera->SetCameraToWorldTransform(lightCameraToWorld);
	}

	// This is the shadow pass.
	this->deviceContext->RSSetViewports(1, &this->shadowPassViewport);
	this->deviceContext->ClearDepthStencilView(this->shadowBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	this->deviceContext->OMSetRenderTargets(0, NULL, this->shadowBufferView);
	this->scene->Render(this->lightSourceCamera.Get(), RenderPass::SHADOW_PASS);
	
	// Not sure if this is necessary, but this will unbind the shadow buffer as a render target so that it can be bound later as a shader resource.
	// Maybe it gets unbound anyway with the next OMSetRenderTargets call below?
	this->deviceContext->OMSetRenderTargets(0, NULL, NULL);

	// This is the main render pass.
	FLOAT backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	this->deviceContext->RSSetViewports(1, &this->mainPassViewport);
	this->deviceContext->ClearRenderTargetView(this->frameBufferView, backgroundColor);
	this->deviceContext->ClearDepthStencilView(this->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	this->deviceContext->OMSetRenderTargets(1, &this->frameBufferView, this->depthStencilView);
	this->scene->Render(this->camera.Get(), RenderPass::MAIN_PASS);

	// This will unbind the the shadow buffer as a shader resource so that it can be bound again as a render target.
	ID3D11ShaderResourceView* shaderResourceViewArray[] = { NULL, NULL };
	ID3D11SamplerState* samplerStateArray[] = { NULL, NULL };
	this->deviceContext->PSSetShaderResources(0, 2, shaderResourceViewArray);
	this->deviceContext->PSSetSamplers(0, 2, samplerStateArray);

	this->swapChain->Present(1, 0);
}

/*virtual*/ LRESULT Game::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
				DestroyWindow(this->mainWindowHandle);

			Reference<ConsoleRenderObject> console;
			if (this->scene->FindRenderObject("console", console))
				console->OnKeyDown(wParam, lParam);

			break;
		}
		case WM_KEYUP:
		{
			if (wParam == VK_F1)
				this->collisionSystemDebugDrawFlags ^= IMZADI_DRAW_FLAG_SHAPES;
			else if (wParam == VK_F2)
				this->collisionSystemDebugDrawFlags ^= IMZADI_DRAW_FLAG_SHAPE_BOXES;
			else if (wParam == VK_F3)
				this->collisionSystemDebugDrawFlags ^= IMZADI_DRAW_FLAG_AABB_TREE;
			else if (wParam == VK_F4)
				AnimatedMeshInstance::SetRenderSkeletons(!AnimatedMeshInstance::GetRenderSkeletons());
			else if (wParam == VK_F5)
				this->ToggleFPSDisplay();
			else if (wParam == VK_F6)
				this->ToggleCollisionStats();
			else if (wParam == VK_F7)
				this->ToggleConsole();
			else if (wParam == VK_F8)
				this->debugDrawVisibilityBoxes = !debugDrawVisibilityBoxes;
			else if (wParam == VK_F9)
			{
				std::vector<Level*> foundLevelsArray;
				this->FindAllEntitiesOfType<Level>(foundLevelsArray);
				for (auto level : foundLevelsArray)
					level->debugDrawNavGraph = !level->debugDrawNavGraph;
			}
			break;
		}
		case WM_INPUT:
		{
			this->inputSystem.HandleInputMessage(wParam, lParam);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_SIZE:
		{
			this->NotifyWindowResized();
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
		assert(game->mainWindowHandle == windowHandle);
		return game->WndProc(msg, wParam, lParam);
	}

	return DefWindowProc(windowHandle, msg, wParam, lParam);
}

void Game::ToggleFPSDisplay()
{
	this->ToggleRenderObject("fps", []() -> RenderObject*
	{
		auto fpsText = new FPSRenderObject();
		fpsText->SetFont("UbuntuMono_R");
		return fpsText;
	});
}

void Game::ToggleCollisionStats()
{
	this->ToggleRenderObject("collision_stats", []() -> RenderObject*
	{
		auto collisionStats = new CollisionStatsRenderObject();
		collisionStats->SetFont("UbuntuMono_R");
		return collisionStats;
	});
}

void Game::ToggleConsole()
{
	this->ToggleRenderObject("console", []() -> RenderObject*
	{
		auto console = new ConsoleRenderObject();
		console->SetFont("UbuntuMono_R");
		return console;
	});
}

void Game::ToggleRenderObject(const std::string& name, std::function<RenderObject*()> renderObjectCreatorFunc)
{
	if (!this->scene)
		return;

	Reference<RenderObject> renderObj;
	if (this->scene->FindRenderObject(name, renderObj))
		this->scene->RemoveRenderObject(name);
	else
	{
		renderObj = renderObjectCreatorFunc();
		renderObj->SetName(name);
		this->scene->AddRenderObject(renderObj);
	}
}

Input* Game::GetController(const std::string& controllerUser)
{
	return this->inputSystem.GetInput(0, controllerUser);
}

void Game::PushControllerUser(const std::string& controllerUser)
{
	this->inputSystem.PushUser(0, controllerUser);
}

std::string Game::GetControllerUser()
{
	return this->inputSystem.GetCurrentUser(0);
}

std::string Game::PopControllerUser()
{
	return this->inputSystem.PopUser(0);
}

/*virtual*/ void Game::Tick(TickPass tickPass)
{
	this->AdvanceEntities(tickPass);

	if (tickPass == TickPass::PARALLEL_WORK)
	{
		this->eventSystem.DispatchAllPendingEvents();
		this->scene->PrepareRenderObjects();
	}
}

/*virtual*/ bool Game::PreShutdown()
{
	return true;
}

/*virtual*/ bool Game::PostShutdown()
{
	return true;
}

void Game::ShutdownAllEntities()
{
	this->spawnedEntityQueue.clear();

	tickingEntityList.sort([](const Entity* entityA, const Entity* entityB) -> bool {
		return entityA->ShutdownOrder() < entityB->ShutdownOrder();
	});

	while (this->tickingEntityList.size() > 0)
	{
		std::list<Reference<Entity>>::iterator iter = this->tickingEntityList.begin();
		Entity* entity = *iter;
		entity->Shutdown();
		this->tickingEntityList.erase(iter);
	}
}

/*virtual*/ bool Game::Shutdown()
{
	this->PreShutdown();

	// Make sure audio stops before all audio assets are freed.
	this->audioSystem.Shutdown();

	this->inputSystem.Shutdown();

	this->eventSystem.Clear();

	this->ShutdownAllEntities();

	this->debugLines.Reset();

	if (this->scene)
	{
		this->scene->Clear();
		this->scene.Reset();
	}

	if (this->assetCache)
	{
		this->assetCache->Clear();
		this->assetCache.Reset();
	}

	this->rasterStateCache.ClearCache();
	this->depthStencilStateCache.ClearCache();
	this->blendStateCache.ClearCache();

	SafeRelease(this->generalSamplerState);
	SafeRelease(this->shadowBufferView);
	SafeRelease(this->shadowBufferViewForShader);
	SafeRelease(this->device);
	SafeRelease(this->deviceContext);
	SafeRelease(this->swapChain);
	SafeRelease(this->frameBufferView);
	SafeRelease(this->depthStencilView);

	if (this->mainWindowHandle)
	{
		DestroyWindow(this->mainWindowHandle);
		this->mainWindowHandle = nullptr;
	}

	UnregisterClass(IMZADI_GAME_WINDOW_CLASS_NAME, this->instance);

	this->PostShutdown();

	return true;
}

double Game::GetAspectRatio() const
{
	double aspectRatio = double(this->mainPassViewport.Width) / double(this->mainPassViewport.Height);
	return aspectRatio;
}

double Game::GetDeltaTime() const
{
	return this->deltaTimeSeconds;
}