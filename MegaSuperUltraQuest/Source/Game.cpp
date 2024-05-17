#include "Game.h"
#include "Scene.h"
#include "RenderMesh.h"
#include "Camera.h"
#include "Math/Transform.h"
#include <format>

using namespace Collision;

Game* Game::gameSingleton = nullptr;

Game::Game(HINSTANCE instance)
{
	this->instance = instance;
	this->mainWindowHandle = NULL;
	this->keepRunning = false;
	this->device = NULL;
	this->deviceContext = NULL;
	this->swapChain = NULL;
	this->frameBufferView = NULL;
	this->rasterizerState = NULL;
	this->scene = nullptr;
	this->assetCache = nullptr;
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

	ID3D11Texture2D* backBufferTexture = nullptr;
	result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
	if (FAILED(result))
		return false;

	result = this->device->CreateRenderTargetView(backBufferTexture, NULL, &this->frameBufferView);
	if (FAILED(result))
		return false;

	backBufferTexture->Release();
	backBufferTexture = nullptr;

	this->deviceContext->OMSetRenderTargets(1, &this->frameBufferView, nullptr);

	D3D11_RASTERIZER_DESC rasterizerDesc;
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

	this->deviceContext->RSSetState(rasterizerState);

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

	this->assetCache.Set(new AssetCache());
	this->assetCache->SetAssetFolder("E:\\ENG_DEV\\CollisionSystem\\MegaSuperUltraQuest\\Assets");	// TODO: Need to get this a different way, obviously.

	this->scene.Set(new Scene());

	Reference<Camera> camera(new Camera());
	this->scene->SetCamera(camera);
	
	Transform cameraTransform;
	cameraTransform.matrix.SetIdentity();
	cameraTransform.translation.SetComponents(0.0, 0.0, 10.0);
	camera->SetCameraToWorldTransform(cameraTransform);

	// Test code: Can we just load a triangle render mesh asset and then draw it?
	Reference<Asset> renderMeshAsset;
	if (this->assetCache->GrabAsset("Models/Box/Box.render_mesh", renderMeshAsset))
	{
		Reference<RenderObject> renderMesh;
		if (renderMeshAsset->MakeRenderInstance(renderMesh))
			this->scene->AddRenderObject(renderMesh);
	}

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
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		FLOAT backgroundColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		this->deviceContext->ClearRenderTargetView(this->frameBufferView, backgroundColor);

		this->scene->Render();

		this->swapChain->Present(1, 0);
	}

	return true;
}

LRESULT Game::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// TODO: We might handle resize at some point by recreating the swap-chain, but
		//       I think that's pretty low priority for now.
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				// TODO: Why can't I just post a quit message?  Why doesn't that work?
				this->keepRunning = false;
			}
			break;
		}
		case WM_DESTROY:
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

	if (this->mainWindowHandle)
	{
		DestroyWindow(this->mainWindowHandle);
		this->mainWindowHandle = nullptr;
	}

	UnregisterClass(GAME_WINDOW_CLASS_NAME, this->instance);

	return true;
}