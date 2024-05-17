#pragma once

#include "AssetCache.h"
#include <d3d11.h>

/**
 * Instances of this class can represent an index or vertex buffer.
 */
class Buffer : public Asset
{
public:
	Buffer();
	virtual ~Buffer();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;

private:
	ID3D11Buffer* buffer;
	UINT numElements;
	UINT strideBytes;
};