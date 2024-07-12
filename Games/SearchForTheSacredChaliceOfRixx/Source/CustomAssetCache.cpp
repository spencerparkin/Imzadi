#include "CustomAssetCache.h"
#include "Assets/DialogData.h"

CustomAssetCache::CustomAssetCache()
{
}

/*virtual*/ CustomAssetCache::~CustomAssetCache()
{
}

/*virtual*/ Imzadi::Asset* CustomAssetCache::CreateBlankAssetForFileType(const std::string& assetFile)
{
	Imzadi::Asset* asset = AssetCache::CreateBlankAssetForFileType(assetFile);
	if (asset)
		return asset;

	std::filesystem::path assetPath(assetFile);
	std::string ext = assetPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
	if (ext == ".dialog")
		return new DialogData();

	return nullptr;
}