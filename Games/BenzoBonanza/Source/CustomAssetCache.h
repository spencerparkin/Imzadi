#pragma once

#include "AssetCache.h"

class CustomAssetCache : public Imzadi::AssetCache
{
public:
	CustomAssetCache();
	virtual ~CustomAssetCache();

protected:
	virtual Imzadi::Asset* CreateBlankAssetForFileType(const std::string& assetFile) override;
};