#pragma once

#include "rapidjson/document.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

class Asset;

/**
 * This is supposed to be one-stop-shopping for any asset we'd want to load,
 * whether that be a render-mesh, texture, shader, etc.
 */
class AssetCache
{
public:
	AssetCache();
	virtual ~AssetCache();

	bool GrabAsset(const std::string& assetFile, std::shared_ptr<Asset>& asset);

	/**
	 * Remove all assets from this cache.
	 */
	void Clear();

private:

	bool ResolveAssetPath(std::string& assetFile);

	std::filesystem::path assetsFolder;

	typedef std::unordered_map<std::string, std::shared_ptr<Asset>> AssetMap;
	AssetMap assetMap;
};

/**
 * This is the base class for all asset types.
 */
class Asset
{
public:
	Asset();
	virtual ~Asset();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) = 0;
	virtual bool Unload() = 0;
};