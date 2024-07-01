#pragma once

#include "AssetCache.h"
#include <d3d12.h>

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

#if 0
		ID3D11Texture2D* GetTexture() { return this->texture; }
		ID3D11ShaderResourceView* GetTextureView() { return this->textureView; }
#endif

	private:
		uint32_t CalcUncompressedTextureSize(uint32_t numMips, uint32_t texelSize, uint32_t textureWidth, uint32_t textureHeight);

#if 0
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* textureView;
#endif
	};
}