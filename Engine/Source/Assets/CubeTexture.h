#pragma once

#include "AssetCache.h"
#include "Texture.h"
#include <d3d11.h>

namespace Imzadi
{
	/**
	 * These can be used for sky-domes or environment/reflection mapping.
	 * Note that you use a special intrinsic to sample from these in the HLSL.
	 * I need to look that up.
	 */
	class IMZADI_API CubeTexture : public Asset
	{
	public:
		CubeTexture();
		virtual ~CubeTexture();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		ID3D11Texture2D* GetTexture() { return this->cubeTexture; }
		ID3D11ShaderResourceView* GetTextureView() { return this->cubeTextureView; }
		ID3D11SamplerState* GetSamplerState() { return this->samplerState; }

	private:
		bool Load(std::vector<Reference<Texture>>& textureArray);

		ID3D11Texture2D* cubeTexture;
		ID3D11ShaderResourceView* cubeTextureView;
		ID3D11SamplerState* samplerState;
	};
}