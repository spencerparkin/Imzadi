#pragma once

#include "AssetCache.h"
#include <d3d11.h>

namespace Imzadi
{
	/**
	 * These can be diffuse textures, alpha maps, bump maps, reflection maps, etc.
	 */
	class IMZADI_API Texture : public Asset
	{
	public:
		Texture();
		virtual ~Texture();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		ID3D11Texture2D* GetTexture() { return this->texture; }
		ID3D11ShaderResourceView* GetTextureView() { return this->textureView; }

	private:
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* textureView;
	};
}