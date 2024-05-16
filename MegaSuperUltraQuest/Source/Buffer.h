#pragma once

#include "AssetCache.h"

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
};