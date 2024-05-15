#pragma once

#include "AssetCache.h"

class Shader : public Asset
{
public:
	Shader();
	virtual ~Shader();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
};