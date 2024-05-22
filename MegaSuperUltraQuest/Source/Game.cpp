#include "Game.h"
#include "Scene.h"
#include "RenderMesh.h"
#include "Camera.h"
#include "Math/Transform.h"
#include <format>
#include <math.h>

using namespace Collision;

Game* Game::gameSingleton = nullptr;

Game::Game(HINSTANCE instance)
{
	this->instance = instance;
	this->mainWindowHandle = NULL;
	this->keepRunning = false;
	this->windowResized = false;
	this->device = NULL;
	this->deviceContext = NULL;
	this->swapChain = NULL;
	this->frameBufferView = NULL;
	this->depthStencilView = NULL;
	this->rasterizerState = NULL;
	this->depthStencilState = NULL;
	this->scene = nullptr;
	this->assetCache = nullptr;
	this->lightParams.lightDirection = Vector3(0.2, -1.0, 0.2).Normalized();
	this->lightParams.lightColor.SetComponents(1.0, 1.0, 1.0, 1.0);
	this->lightParams.directionalLightIntensity = 1.0;
	this->lightParams.ambientLightIntensity = 0.02;
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
	{
		DWORD errorCode = GetLastError();
		std::string errorMsg = std::format("Failed to register window class!  Error code: {}", errorCode);
		MessageBox(NULL, errorMsg.c_str(), TEXT("Error!"), MB_OK);
		return false;
	}

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
		DWORD errorCode = GetLastError();
		std::string errorMsg = std::format("Failed to create main window!  Error code: {}", errorCode);
		MessageBox(NULL, errorMsg.c_str(), TEXT("Error!"), MB_OK);
		return false;
	}

	SetWindowLongPtr(this->mainWindowHandle, GWLP_USERDATA, LONG_PTR(this));

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
		DWORD errorCode = GetLastError();
		std::string errorMsg = std::format("Failed to create D3D11 device, context and swap-chain!  Error code: {}", errorCode);
		MessageBox(NULL, errorMsg.c_str(), TEXT("Error!"), MB_OK);
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
	if (!this->camera->LookAt(Vector3(-20.0, 30.0, 50.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)))
		return false;

	this->windowResized = false;
	if (!this->RecreateViews())
		return false;

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;

	result = this->device->CreateRasterizerState(&rasterizerDesc, &this->rasterizerState);
	if (FAILED(result))
		return false;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = 0;
	depthStencilDesc.StencilWriteMask = 0;

	result = this->device->CreateDepthStencilState(&depthStencilDesc, &this->depthStencilState);
	if (FAILED(result))
		return false;

	this->assetCache.Set(new AssetCache());
	this->assetCache->SetAssetFolder("E:\\ENG_DEV\\CollisionSystem\\MegaSuperUltraQuest\\Assets");	// TODO: Need to get this a different way, obviously.

	this->scene.Set(new Scene());

	this->LoadAndPlaceRenderMesh("Models/Box/Box.render_mesh", Vector3(-10.0, 6.0, 0.0), Quaternion(Vector3(1.0, 1.0, 0.0).Normalized(), M_PI / 4.0));
	this->LoadAndPlaceRenderMesh("Models/Teapot/Teapot.render_mesh", Vector3(10.0, 6.0, 0.0), Quaternion());
	this->LoadAndPlaceRenderMesh("Models/GroundPlane/GroundPlane.render_mesh", Vector3(), Quaternion());
	//this->LoadAndPlaceRenderMesh("Models/Quad/Quad.render_mesh", Vector3(0.0, 0.0, 0.0), Quaternion());

	this->keepRunning = true;
	return true;
}

Reference<RenderObject> Game::LoadAndPlaceRenderMesh(
			const std::string& renderMeshFile,
			const Collision::Vector3& position,
			const Collision::Quaternion& orientation)
{
	Reference<RenderObject> renderMesh;
	Reference<Asset> renderMeshAsset;

	if (this->assetCache->GrabAsset(renderMeshFile, renderMeshAsset))
	{
		if (renderMeshAsset->MakeRenderInstance(renderMesh))
		{
			Transform objectToWorld;
			objectToWorld.matrix.SetFromQuat(orientation);
			objectToWorld.translation = position;

			auto instance = dynamic_cast<RenderMeshInstance*>(renderMesh.Get());
			instance->SetObjectToWorldTransform(objectToWorld);
			this->scene->AddRenderObject(renderMesh);
		}
	}

	return renderMesh;
}

bool Game::RecreateViews()
{
	if (this->frameBufferView)
	{
		this->frameBufferView->Release();
		this->frameBufferView = nullptr;
	}

	if (this->depthStencilView)
	{
		this->depthStencilView->Release();
		this->depthStencilView = nullptr;
	}

	HRESULT result = 0;

	if (this->windowResized)
	{
		result = this->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(result))
			return false;
	}

	ID3D11Texture2D* backBufferTexture = nullptr;
	result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
	if (FAILED(result))
		return false;

	result = this->device->CreateRenderTargetView(backBufferTexture, NULL, &this->frameBufferView);
	if (FAILED(result))
		return false;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	backBufferTexture->GetDesc(&depthBufferDesc);

	backBufferTexture->Release();
	backBufferTexture = nullptr;

	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthBuffer = nullptr;
	result = this->device->CreateTexture2D(&depthBufferDesc, NULL, &depthBuffer);
	if (FAILED(result))
		return false;

	result = this->device->CreateDepthStencilView(depthBuffer, NULL, &this->depthStencilView);
	if (FAILED(result))
		return false;

	depthBuffer->Release();
	depthBuffer = nullptr;

	RECT clientRect;
	GetClientRect(this->mainWindowHandle, &clientRect);

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = FLOAT(clientRect.right - clientRect.left);
	viewport.Height = FLOAT(clientRect.bottom - clientRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	this->deviceContext->RSSetViewports(1, &viewport);

	double aspectRatio = double(viewport.Width) / double(viewport.Height);

	Frustum frustum;
	frustum.SetFromAspectRatio(aspectRatio, M_PI / 3.0, 0.1, 1000.0);
	this->camera->SetFrustum(frustum);

	return true;
}

bool Game::Run()
{
	while (this->keepRunning)
	{
		MSG message{};
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
				this->keepRunning = false;

			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (this->windowResized)
		{
			this->deviceContext->OMSetRenderTargets(0, NULL, NULL);
			this->RecreateViews();
			this->windowResized = false;
		}

		this->Render();
	}

	return true;
}

void Game::Render()
{
	FLOAT backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	this->deviceContext->ClearRenderTargetView(this->frameBufferView, backgroundColor);

	this->deviceContext->ClearDepthStencilView(this->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// TODO: Do shadow pass here before the main pass.  I think we would set different render targets and
	//       then call this->scene->render() with a different camera.  This would render into an off-screen
	//       shadow depth map which would then be used in the main pass.

	this->deviceContext->OMSetRenderTargets(1, &this->frameBufferView, this->depthStencilView);

	this->deviceContext->RSSetState(rasterizerState);
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState, 0);

	this->scene->Render(this->camera.Get());

	this->swapChain->Present(1, 0);
}

LRESULT Game::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
				DestroyWindow(this->mainWindowHandle);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_SIZE:
		{
			this->windowResized = true;
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

bool Game::Shutdown()
{
	// TODO: Do we need to wait for the GPU to finish?!

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

	if (this->rasterizerState)
	{
		this->rasterizerState->Release();
		this->rasterizerState = nullptr;
	}

	if (this->depthStencilState)
	{
		this->depthStencilState->Release();
		this->depthStencilState = nullptr;
	}

	if (this->device)
	{
		this->device->Release();
		this->device = nullptr;
	}

	if (this->deviceContext)
	{
		this->deviceContext->Release();
		this->deviceContext = nullptr;
	}

	if (this->swapChain)
	{
		this->swapChain->Release();
		this->swapChain = nullptr;
	}

	if (this->frameBufferView)
	{
		this->frameBufferView->Release();
		this->frameBufferView = nullptr;
	}

	if (this->depthStencilView)
	{
		this->depthStencilView->Release();
		this->depthStencilView = nullptr;
	}

	if (this->mainWindowHandle)
	{
		DestroyWindow(this->mainWindowHandle);
		this->mainWindowHandle = nullptr;
	}

	UnregisterClass(GAME_WINDOW_CLASS_NAME, this->instance);

	return true;
}