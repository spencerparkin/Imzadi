#include "Game.h"
#include "Scene.h"
#include "Assets/RenderMesh.h"
#include "RenderObjects/RenderMeshInstance.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "RenderObjects/TextRenderObject.h"
#include "Camera.h"
#include "Entities/Level.h"
#include "Math/Transform.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Log.h"
#include <format>
#include <math.h>
#include <codecvt>
#include <locale>

using namespace Imzadi;

Game* Game::gameSingleton = nullptr;

Game::Game(HINSTANCE instance) : controller(0)
{
	this->accelerationDuetoGravity = 40.0;
	this->collisionSystemDebugDrawFlags = 0;
	this->deltaTimeSeconds = 0.0;
	this->lastTickTime = 0;
	this->instance = instance;
	this->mainWindowHandle = NULL;
	this->keepRunning = false;
	this->windowResized = false;
	this->scene = nullptr;
	this->assetCache = nullptr;
	this->lightParams.lightDirection = Vector3(0.2, -1.0, 0.2).Normalized();
	this->lightParams.lightColor.SetComponents(1.0, 1.0, 1.0, 1.0);
	this->lightParams.directionalLightIntensity = 1.0;
	this->lightParams.ambientLightIntensity = 0.1;
	this->lightParams.lightCameraDistance = 200.0;
	this->commandData.fenceEvent = INVALID_HANDLE_VALUE;

	for (int i = 0; i < this->numFrames; i++)
	{
		Frame* frame = &this->frameArray[i];
		frame->fenceEvent = INVALID_HANDLE_VALUE;
	}

	ZeroMemory(&this->mainPassViewport, sizeof(D3D12_VIEWPORT));
	ZeroMemory(&this->shadowPassViewport, sizeof(D3D12_VIEWPORT));

	lstrcpy(this->windowTitle, "Imzadi Game Engine");
}

/*virtual*/ Game::~Game()
{
}

/*virtual*/ bool Game::CreateRenderWindow()
{
	WNDCLASSEX winClass;
	::ZeroMemory(&winClass, sizeof(winClass));
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = &Game::WndProcEntryFunc;
	winClass.lpszClassName = IMZADI_GAME_WINDOW_CLASS_NAME;

	if (!RegisterClassEx(&winClass))
	{
		IMZADI_LOG_ERROR("Failed to register window class!  Error code: %d", GetLastError());
		return false;
	}

	RECT rect = { 0, 0, 1024, 768 };
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

	this->assetCache->AddAssetFolder(R"(E:\ENG_DEV\Imzadi\Engine\Assets)");	// TODO: Need to not hard-code a path here.

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

CollisionSystem* Game::GetCollisionSystem()
{
	return &this->collisionSystem;
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

	HRESULT result = 0;

	UINT factoryFlags = 0;

#if defined _DEBUG
	ComPtr<ID3D12Debug> debugController;
	result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to get debug interface with error code: %d", result);
		return false;
	}

	// Without enabling this we don't get useful logging output from DX12 when
	// the API is miss-used or something else goes wrong, I think.
	debugController->EnableDebugLayer();

	// Not sure at the moment why we care about this flag, but okay.
	factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory2> factory2;
	result = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory2));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create DXGI factory2 with error code: %d", result);
		return false;
	}

	ComPtr<IDXGIFactory6> factory6;
	result = factory2->QueryInterface(IID_PPV_ARGS(&factory6));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create DXGI factory6 from factory2 with error code: %d", result);
		return false;
	}

	// Look for a GPU to suit our needs.  We just use the first one we find.
	ComPtr<IDXGIAdapter1> chosenAdapter;
	UINT adapterNumber = 0;
	while (true)
	{
		ComPtr<IDXGIAdapter1> adapter1;
		result = factory6->EnumAdapterByGpuPreference(adapterNumber, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter1));
		if (FAILED(result))
			break;

		DXGI_ADAPTER_DESC1 adapterDesc{};
		adapter1->GetDesc1(&adapterDesc);
		if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
			continue;
		
		// This won't actually create the device, but it will test to see if device creation would succeed.
		result = D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(result))
		{
			chosenAdapter = adapter1.Detach();
			break;
		}

		adapterNumber++;
	}

	if (!chosenAdapter)
	{
		IMZADI_LOG_ERROR("Failed to find a GPU we can use.");
		return false;
	}

	DXGI_ADAPTER_DESC1 chosenAdapterDesc{};
	chosenAdapter->GetDesc1(&chosenAdapterDesc);
	std::string gpuDesc = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(std::wstring(chosenAdapterDesc.Description));
	IMZADI_LOG_INFO("GPU chosen: %s", gpuDesc.c_str());

	result = D3D12CreateDevice(chosenAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&this->device));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create D3D12 device from chosen GPU adapater.  Error: %d", result);
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = this->device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&this->commandData.queue));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create command queue with error: %d", result);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.BufferCount = this->numFrames;
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain1;
	result = factory2->CreateSwapChainForHwnd(this->commandData.queue.Get(), this->mainWindowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create swap-chain with error: %d", result);
		return false;
	}

	result = swapChain1.As(&this->swapChain);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to cast swap-chain.  Error: %d", result);
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc{};
	renderTargetViewHeapDesc.NumDescriptors = this->numFrames;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = this->device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(&this->renderTargetViewHeap));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create RTV descriptor heap with error: %d", result);
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC depthStencilViewHeapDesc{};
	depthStencilViewHeapDesc.NumDescriptors = this->numFrames;
	depthStencilViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	depthStencilViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = this->device->CreateDescriptorHeap(&depthStencilViewHeapDesc, IID_PPV_ARGS(&this->depthStencilViewHeap));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create DSV descriptor heap with error: %d", result);
		return false;
	}

	this->SetCamera(new Camera());
	this->camera->SetViewMode(Camera::ViewMode::PERSPECTIVE);
	if (!this->camera->LookAt(Vector3(-20.0, 30.0, 50.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0)))
		return false;

	// Create synchronization object's we'll need to manage the CPU and GPU time-lines.
	// Specifically, we never want to mess with data on the CPU that the GPU is still using.
	for (int i = 0; i < this->numFrames; i++)
	{
		Frame* frame = &this->frameArray[i];

		result = this->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frame->fence));
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to create fence with error: %d", result);
			return false;
		}

		frame->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!frame->fenceEvent)
		{
			IMZADI_LOG_ERROR("Failed to create fence event with error: %d", GetLastError());
			return false;
		}

		// All frames start life in the completed state.
		frame->targetFenceValue = frame->fence->GetCompletedValue();
	}

	this->windowResized = false;
	if (!this->RecreateViews())
	{
		IMZADI_LOG_ERROR("Initial view creation failed.");
		return false;
	}

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// TODO: Need to add constants buffer visibility, etc.

	ComPtr<ID3DBlob> signature, error;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to serialize root signature with error: %d", result);
		return false;
	}

	result = this->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&this->rootSignature));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create root signature with error: %d", result);
		return false;
	}

	result = this->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->commandData.allocator));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create command allocator with error: %d", result);
		return false;
	}

	// Note that no initial pipeline state is bound here.  We'll do that when we're ready to draw something.
	result = this->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->commandData.allocator.Get(), nullptr, IID_PPV_ARGS(&this->commandData.list));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create initial graphics command list for with error: %d", result);
		return false;
	}

	// Start life in the closed state rather than the recording state.
	this->commandData.list->Close();

	result = this->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->commandData.fence));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create fence for command data with error: %d", result);
		return false;
	}

	this->commandData.fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!this->commandData.fenceEvent)
	{
		IMZADI_LOG_ERROR("Failed to create fence event with error: %d", GetLastError());
		return false;
	}

	Camera::OrthographicParams orthoParams{};
	orthoParams.nearClip = 0.0;
	orthoParams.farClip = 1000.0;
	orthoParams.width = 400.0;
	orthoParams.height = 400.0;

	this->lightSourceCamera.Set(new Camera());
	this->lightSourceCamera->SetViewMode(Camera::ViewMode::ORTHOGRAPHIC);
	this->lightSourceCamera->SetOrthographicParams(orthoParams);

#if 0
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
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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
#endif

	if (!this->PostInit())
	{
		IMZADI_LOG_ERROR("Post initialization failed.");
		return false;
	}

	this->keepRunning = true;
	return true;
}

Game::Frame* Game::GetCurrentFrame()
{
	UINT frameIndex = this->swapChain->GetCurrentBackBufferIndex();
	IMZADI_ASSERT(0 <= frameIndex && frameIndex < this->numFrames);
	return &this->frameArray[frameIndex];
}

bool Game::IsFrameComplete(Frame* frame)
{
	UINT64 fenceValue = frame->fence->GetCompletedValue();
	return fenceValue == frame->targetFenceValue;
}

void Game::StallUntilFrameComplete(Frame* frame)
{
	if (!this->IsFrameComplete(frame))
	{
		WaitForSingleObjectEx(frame->fenceEvent, INFINITE, FALSE);
		IMZADI_ASSERT(this->IsFrameComplete(frame));
	}
}

void Game::StallUntilAllFramesComplete()
{
	for (int i = 0; i < this->numFrames; i++)
		this->StallUntilFrameComplete(&this->frameArray[i]);
}

Reference<RenderObject> Game::LoadAndPlaceRenderMesh(const std::string& renderMeshFile, const Vector3& position, const Quaternion& orientation)
{
	Reference<RenderObject> renderMesh;
	Reference<Asset> renderMeshAsset;

	if (this->assetCache->LoadAsset(renderMeshFile, renderMeshAsset))
	{
		if (renderMeshAsset->MakeRenderInstance(renderMesh))
		{
			auto instance = dynamic_cast<RenderMeshInstance*>(renderMesh.Get());
			if (instance)
			{
				Transform objectToWorld;
				objectToWorld.matrix.SetFromQuat(orientation);
				objectToWorld.translation = position;
				instance->SetObjectToWorldTransform(objectToWorld);
			}

			this->scene->AddRenderObject(renderMesh);
		}
	}

	return renderMesh;
}

bool Game::RecreateViews()
{
	HRESULT result = 0;

	this->StallUntilAllFramesComplete();

	for (int i = 0; i < this->numFrames; i++)
	{
		Frame* frame = &this->frameArray[i];
		frame->renderTarget.Reset();
		frame->depthStencil.Reset();
	}

	if (this->windowResized)
	{
		result = this->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to resize swap-chain according to new window size.  Error code: %d", result);
			return false;
		}
	}

	RECT clientRect;
	GetClientRect(this->mainWindowHandle, &clientRect);

	this->mainPassViewport.TopLeftX = 0.0f;
	this->mainPassViewport.TopLeftY = 0.0f;
	this->mainPassViewport.Width = FLOAT(clientRect.right - clientRect.left);
	this->mainPassViewport.Height = FLOAT(clientRect.bottom - clientRect.top);
	this->mainPassViewport.MinDepth = 0.0f;
	this->mainPassViewport.MaxDepth = 1.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = this->renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	UINT renderTargetViewDescriptorSize = this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilViewHandle = this->depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart();
	UINT depthStencilViewDescriptorSize = this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_RESOURCE_DESC depthStencilResourceDesc{};
	depthStencilResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilResourceDesc.Alignment = 0;
	depthStencilResourceDesc.Width = mainPassViewport.Width;
	depthStencilResourceDesc.Height = mainPassViewport.Height;
	depthStencilResourceDesc.DepthOrArraySize = 1;
	depthStencilResourceDesc.MipLevels = 1;
	depthStencilResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilResourceDesc.SampleDesc.Count = 1;
	depthStencilResourceDesc.SampleDesc.Quality = 0;
	depthStencilResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES depthStencilHeapProperties{};
	depthStencilHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthStencilHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthStencilHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE depthStencilClearValue{};
	depthStencilClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilClearValue.DepthStencil.Depth = 1.0f;
	depthStencilClearValue.DepthStencil.Stencil = 0;

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	for (int i = 0; i < this->numFrames; i++)
	{
		Frame* frame = &this->frameArray[i];

		result = this->swapChain->GetBuffer(i, IID_PPV_ARGS(&frame->renderTarget));
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to get swap chain buffer %d with error: %d", i, result);
			return false;
		}

		this->device->CreateRenderTargetView(frame->renderTarget.Get(), nullptr, renderTargetViewHandle);
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

		result = this->device->CreateCommittedResource(
										&depthStencilHeapProperties,
										D3D12_HEAP_FLAG_NONE,
										&depthStencilResourceDesc,
										D3D12_RESOURCE_STATE_DEPTH_WRITE,
										&depthStencilClearValue,
										IID_PPV_ARGS(&frame->depthStencil));
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to create depth stencil buffer for frame %d with error: %d", i, result);
			return false;
		}

		this->device->CreateDepthStencilView(frame->depthStencil.Get(), &depthStencilViewDesc, depthStencilViewHandle);
		depthStencilViewHandle.ptr += depthStencilViewDescriptorSize;
	}

	IMZADI_LOG_INFO("Viewport dimensions: %d x %d", int(this->mainPassViewport.Width), int(this->mainPassViewport.Height));

	double aspectRatio = double(this->mainPassViewport.Width) / double(this->mainPassViewport.Height);

	Frustum frustum;
	frustum.SetFromAspectRatio(aspectRatio, M_PI / 3.0, 0.1, 1000.0);
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
	if (this->lastTickTime == 0)
		this->lastTickTime = ::clock();

	clock_t currentTickTime = ::clock();
	clock_t deltaTickTime = currentTickTime - this->lastTickTime;
	this->deltaTimeSeconds = double(deltaTickTime) / double(CLOCKS_PER_SEC);
	this->lastTickTime = currentTickTime;

	// This can be useful while debugging, but I'm also doing this because
	// I'm seeing the first frame's delta-time be way too big and this can
	// cause the character to move too far in the first frame and then tunnel
	// through the ground.
	if (this->deltaTimeSeconds >= 0.1)
		return true;

	this->debugLines->Clear();

	this->PumpWindowsMessages();

	this->controller.Update();

	// Can initiate collision queries in this pass.
	this->Tick(TickPass::PRE_TICK);

	// Do work that runs in parallel with the collision system.  (e.g., animating skeletons and performing skinning.)
	this->Tick(TickPass::MID_TICK);

	// Stall waiting for the collision system to complete all queries and commands.
	this->collisionSystem.FlushAllTasks();

	// Collision queries can now be acquired and used in this pass.  (e.g., to solve constraints.)
	this->Tick(TickPass::POST_TICK);

	if (this->windowResized)
	{
		this->RecreateViews();
		this->windowResized = false;
	}

	if (this->collisionSystemDebugDrawFlags != 0)
	{
		auto query = new DebugRenderQuery();
		query->SetDrawFlags(this->collisionSystemDebugDrawFlags);
		TaskID collisionSystemDebugDrawTaskID = 0;
		this->collisionSystem.MakeQuery(query, collisionSystemDebugDrawTaskID);
		this->collisionSystem.FlushAllTasks();
		Result* result = this->collisionSystem.ObtainQueryResult(collisionSystemDebugDrawTaskID);
		if (result)
		{
			auto debugRenderResult = dynamic_cast<DebugRenderResult*>(result);
			if (debugRenderResult)
				for (const DebugRenderResult::RenderLine& line : debugRenderResult->GetRenderLineArray())
					this->debugLines->AddLine({ line.color, line.line });

			this->collisionSystem.Free<Result>(result);
		}
	}

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

void Game::AdvanceEntities(TickPass tickPass)
{
	while (this->spawnedEntityQueue.size() > 0)
	{
		std::list<Reference<Entity>>::iterator iter = this->spawnedEntityQueue.begin();
		Reference<Entity> entity = *iter;
		this->spawnedEntityQueue.erase(iter);
		if (entity->Setup())
			this->tickingEntityList.push_back(entity);
	}

	std::list<Reference<Entity>>::iterator iter = this->tickingEntityList.begin();
	while (iter != this->tickingEntityList.end())
	{
		std::list<Reference<Entity>>::iterator nextIter(iter);
		nextIter++;

		Entity* entity = *iter;

		if (!entity->Tick(tickPass, this->deltaTimeSeconds))
		{
			entity->Shutdown(false);
			this->tickingEntityList.erase(iter);
		}

		iter = nextIter;
	}
}

void Game::EnqueuePreRenderCallback(PreRenderCallback callback)
{
	this->preRenderCallbackQueue.push_back(callback);
}

/*virtual*/ void Game::Render()
{
	Vector3 lightCameraPosition = this->camera->GetEyePoint() - this->lightParams.lightCameraDistance * this->lightParams.lightDirection;
	if (!this->lightSourceCamera->LookAt(lightCameraPosition, this->camera->GetEyePoint(), Vector3(0.0, 1.0, 0.0)))
	{
		Transform lightCameraToWorld;
		lightCameraToWorld.translation = lightCameraPosition;
		lightCameraToWorld.matrix.SetColumnVectors(Vector3(1.0, 0.0, 0.0), Vector3(0.0, 0.0, -1.0), Vector3(0.0, 1.0, 0.0));
		this->lightSourceCamera->SetCameraToWorldTransform(lightCameraToWorld);
	}

#if 0
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
#endif

	HRESULT result = 0;

	Frame* frame = this->GetCurrentFrame();
	this->StallUntilFrameComplete(frame);
	if (frame->targetFenceValue > 0)
		this->swapChain->Present(1, 0);

	frame = this->GetCurrentFrame();
	IMZADI_ASSERT(this->IsFrameComplete(frame));

	UINT frameIndex = this->swapChain->GetCurrentBackBufferIndex();

	this->commandData.allocator->Reset();
	this->commandData.list->Reset(this->commandData.allocator.Get(), nullptr);

	while (this->preRenderCallbackQueue.size() > 0)
	{
		std::list<PreRenderCallback>::iterator iter = this->preRenderCallbackQueue.begin();
		PreRenderCallback callback = *iter;
		this->preRenderCallbackQueue.erase(iter);
		callback(this->commandData.list.Get());
	}

	D3D12_RESOURCE_BARRIER barrierPresentToTarget{};
	barrierPresentToTarget.Transition.pResource = frame->renderTarget.Get();
	barrierPresentToTarget.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierPresentToTarget.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	this->commandData.list->ResourceBarrier(1, &barrierPresentToTarget);

	this->commandData.list->SetGraphicsRootSignature(this->rootSignature.Get());
	this->commandData.list->RSSetViewports(1, &this->mainPassViewport);
	
	UINT renderTargetViewDescriptorSize = this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = this->renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetViewHandle.ptr += frameIndex * renderTargetViewDescriptorSize;

	UINT depthStencilViewDescriptorSize = this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilViewHandle = this->depthStencilViewHeap->GetCPUDescriptorHandleForHeapStart();
	depthStencilViewHandle.ptr += frameIndex * depthStencilViewDescriptorSize;

	this->commandData.list->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	this->commandData.list->ClearRenderTargetView(renderTargetViewHandle, clearColor, 0, nullptr);

	this->commandData.list->ClearDepthStencilView(depthStencilViewHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	this->scene->Render(this->camera.Get(), RenderPass::MAIN_PASS);

	D3D12_RESOURCE_BARRIER barrierTargetToPresent{};
	barrierTargetToPresent.Transition.pResource = frame->renderTarget.Get();
	barrierTargetToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierTargetToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	this->commandData.list->ResourceBarrier(1, &barrierTargetToPresent);

	result = this->commandData.list->Close();
	if (FAILED(result))
	{
		IMZADI_LOG_FATAL_ERROR("Failed to close command list with error: %d", result);
	}

	ID3D12CommandList* commandListArray[] = { this->commandData.list.Get() };
	this->commandData.queue->ExecuteCommandLists(_countof(commandListArray), commandListArray);

	// Lastly, schedule a signal to occur on the GPU to tell us (on the CPU) when the frame is done.
	ResetEvent(frame->fenceEvent);
	frame->targetFenceValue++;
	frame->fence->SetEventOnCompletion(frame->targetFenceValue, frame->fenceEvent);
	this->commandData.queue->Signal(frame->fence.Get(), frame->targetFenceValue);

	// Note that we don't stall here waiting for the frame to complete.
	// Rather, we want to do a bunch of work in parallel with the GPU.
	// That's why we stall at the start of the next render.
}

/*virtual*/ LRESULT Game::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
				DestroyWindow(this->mainWindowHandle);
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
	if (!this->scene)
		return;

	Reference<RenderObject> renderObj;
	if (this->scene->FindRenderObject("fps", renderObj))
		this->scene->RemoveRenderObject("fps");
	else
	{
		auto fpsText = new FPSRenderObject();
		fpsText->SetFont("UbuntuMono_R");
		this->scene->AddRenderObject("fps", fpsText);
	}
}

Controller* Game::GetController(const std::string& controllerUser)
{
	if (controllerUser == this->GetControllerUser())
		return &this->controller;

	return nullptr;
}

void Game::PushControllerUser(const std::string& controllerUser)
{
	if (this->GetControllerUser() != controllerUser)
		this->controllerUserStack.push_back(controllerUser);
}

std::string Game::GetControllerUser()
{
	if (this->controllerUserStack.size() == 0)
		return "";

	return this->controllerUserStack[this->controllerUserStack.size() - 1];
}

std::string Game::PopControllerUser()
{
	std::string controllerUser = this->GetControllerUser();
	if (this->controllerUserStack.size() > 0)
		this->controllerUserStack.pop_back();
	return controllerUser;
}

/*virtual*/ void Game::Tick(TickPass tickPass)
{
	this->AdvanceEntities(tickPass);

	if (tickPass == TickPass::MID_TICK)
	{
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

/*virtual*/ bool Game::Shutdown()
{
	this->StallUntilAllFramesComplete();

	this->PreShutdown();

	// TODO: Where's the leak?  DX12 says I'm leaking some resources.  :(

	this->spawnedEntityQueue.clear();

	while (this->tickingEntityList.size() > 0)
	{
		std::list<Reference<Entity>>::iterator iter = this->tickingEntityList.begin();
		Entity* entity = *iter;
		entity->Shutdown(true);
		this->tickingEntityList.erase(iter);
	}

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

	for (int i = 0; i < this->numFrames; i++)
	{
		Frame* frame = &this->frameArray[i];
		frame->renderTarget.Reset();
		frame->depthStencil.Reset();
		CloseHandle(frame->fenceEvent);
	}

	this->device.Reset();
	this->swapChain.Reset();
	this->rootSignature.Reset();
	this->renderTargetViewHeap.Reset();
	this->depthStencilViewHeap.Reset();
	this->commandData.allocator.Reset();
	this->commandData.list.Reset();
	this->commandData.queue.Reset();
	this->commandData.fence.Reset();
	CloseHandle(this->commandData.fenceEvent);

	if (this->mainWindowHandle)
	{
		DestroyWindow(this->mainWindowHandle);
		this->mainWindowHandle = nullptr;
	}

	UnregisterClass(IMZADI_GAME_WINDOW_CLASS_NAME, this->instance);

	this->PostShutdown();

	return true;
}