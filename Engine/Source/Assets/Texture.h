#pragma once

#include "AssetCache.h"
#include <d3d11.h>

namespace Imzadi
{
	class IMZADI_API Texture : public Asset
	{
	public:
		Texture();
		virtual ~Texture();

		virtual bool Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache) override;
		virtual bool Unload() override;

		ID3D11ShaderResourceView* GetTextureView() { return this->textureView; }
		ID3D11SamplerState* GetSamplerState() { return this->samplerState; }

	private:
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* textureView;
		ID3D11SamplerState* samplerState;
	};
}