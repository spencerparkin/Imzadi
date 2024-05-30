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

	ID3D11Buffer* GetBuffer() { return this->buffer; }
	UINT GetStride() { return this->strideBytes; }
	UINT GetNumElements() { return this->numElements; }
	DXGI_FORMAT GetFormat() { return this->componentFormat; }

private:
	ID3D11Buffer* buffer;			///< A pointer to the DX11 buffer interface.
	UINT numElements;				///< Here, an "element" refers to a vertex or an index.
	UINT strideBytes;				///< This is the byte-distance in the buffer from the start of one element to another.
	DXGI_FORMAT componentFormat;	///< This is the format of each component of an element.
};