#pragma once

#include "Buffer.h"
#include <d3d12.h>

namespace Imzadi
{
	/**
	 * These can be diffuse textures, alpha maps, bump maps, reflection maps, etc.
	 */
	class IMZADI_API Texture : public Buffer
	{
	public:
		Texture();
		virtual ~Texture();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;

	private:

		virtual bool GetResourceDesc(D3D12_RESOURCE_DESC& resourceDesc) override;

		D3D12_RESOURCE_DESC textureResourceDesc;
	};
}