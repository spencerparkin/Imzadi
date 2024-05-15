#pragma once

#include "AssetCache.h"

class Texture : public Asset
{
public:
	Texture();
	virtual ~Texture();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
};