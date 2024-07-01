#include "StateCache.h"
#include "Game.h"
#include "Log.h"

using namespace Imzadi;

#if 0

//----------------------------- StateCache<ID3D11RasterizerState, D3D11_RASTERIZER_DESC> -----------------------------

/*virtual*/ void* StateCache<ID3D11RasterizerState, D3D11_RASTERIZER_DESC>::CreateState(const void* desc)
{
	ID3D11Device* device = Game::Get()->GetDevice();

	auto rasterizerDesc = static_cast<const D3D11_RASTERIZER_DESC*>(desc);

	ID3D11RasterizerState* rasterizerState = nullptr;
	HRESULT result = device->CreateRasterizerState(rasterizerDesc, &rasterizerState);
	if (FAILED(result))
	{
		IMZADI_LOG_FATAL_ERROR("Failed to create rasterizer state with error code: %d", result);
	}

	return rasterizerState;
}

/*virtual*/ void StateCache<ID3D11RasterizerState, D3D11_RASTERIZER_DESC>::ApplyState(void* state)
{
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();
	auto rasterizerState = static_cast<ID3D11RasterizerState*>(state);
	deviceContext->RSSetState(rasterizerState);
}

/*virtual*/ void StateCache<ID3D11RasterizerState, D3D11_RASTERIZER_DESC>::SetState(const void* desc)
{
	this->guts.SetState(static_cast<const D3D11_RASTERIZER_DESC*>(desc));
}

/*virtual*/ void StateCache<ID3D11RasterizerState, D3D11_RASTERIZER_DESC>::ClearCache()
{
	this->guts.ClearCache();
}

//----------------------------- StateCache<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC> -----------------------------

/*virtual*/ void* StateCache<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC>::CreateState(const void* desc)
{
	ID3D11Device* device = Game::Get()->GetDevice();

	auto depthStencilDesc = static_cast<const D3D11_DEPTH_STENCIL_DESC*>(desc);

	ID3D11DepthStencilState* depthStencilState = nullptr;
	HRESULT result = device->CreateDepthStencilState(depthStencilDesc, &depthStencilState);
	if (FAILED(result))
	{
		IMZADI_LOG_FATAL_ERROR("Failed to create depth stencil state with error code: %d", result);
	}

	return depthStencilState;
}

/*virtual*/ void StateCache<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC>::ApplyState(void* state)
{
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();
	auto depthStencilState = static_cast<ID3D11DepthStencilState*>(state);
	deviceContext->OMSetDepthStencilState(depthStencilState, 0);
}

/*virtual*/ void StateCache<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC>::SetState(const void* desc)
{
	this->guts.SetState(static_cast<const D3D11_DEPTH_STENCIL_DESC*>(desc));
}

/*virtual*/ void StateCache<ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC>::ClearCache()
{
	this->guts.ClearCache();
}

//----------------------------- StateCache<ID3D11BlendState, D3D11_BLEND_DESC> -----------------------------

/*virtual*/ void* StateCache<ID3D11BlendState, D3D11_BLEND_DESC>::CreateState(const void* desc)
{
	ID3D11Device* device = Game::Get()->GetDevice();

	auto blendStateDesc = static_cast<const D3D11_BLEND_DESC*>(desc);

	ID3D11BlendState* blendState = nullptr;
	HRESULT result = device->CreateBlendState(blendStateDesc, &blendState);
	if (FAILED(result))
	{
		IMZADI_LOG_FATAL_ERROR("Failed to create blend state with error code: %d", result);
	}

	return blendState;
}

/*virtual*/ void StateCache<ID3D11BlendState, D3D11_BLEND_DESC>::ApplyState(void* state)
{
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();
	auto blendState = static_cast<ID3D11BlendState*>(state);
	deviceContext->OMSetBlendState(blendState, NULL, 0xFFFFFFFF);
}

/*virtual*/ void StateCache<ID3D11BlendState, D3D11_BLEND_DESC>::SetState(const void* desc)
{
	this->guts.SetState(static_cast<const D3D11_BLEND_DESC*>(desc));
}

/*virtual*/ void StateCache<ID3D11BlendState, D3D11_BLEND_DESC>::ClearCache()
{
	this->guts.ClearCache();
}

#endif