#include "CustomAssetCache.h"
#include "Assets/DialogData.h"
#include "Assets/GameProgress.h"
#include "Assets/ZipLine.h"
#include "Assets/GameLevelData.h"
#include "Assets/RubiksCubieData.h"
#include "Assets/SlidingDoorData.h"

CustomAssetCache::CustomAssetCache()
{
}

/*virtual*/ CustomAssetCache::~CustomAssetCache()
{
}

/*virtual*/ Imzadi::Asset* CustomAssetCache::CreateBlankAssetForFileType(const std::string& assetFile)
{
	std::filesystem::path assetPath(assetFile);
	std::string ext = assetPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
	if (ext == ".dialog")
		return new DialogData();
	else if (ext == ".progress")
		return new GameProgress();
	else if (ext == ".zip_line")
		return new ZipLine();
	else if (ext == ".level")
		return new GameLevelData();
	else if (ext == ".cubie")
		return new RubiksCubieData();
	else if (ext == ".door")
		return new SlidingDoorData();

	Imzadi::Asset* asset = AssetCache::CreateBlankAssetForFileType(assetFile);
	if (asset)
		return asset;

	return nullptr;
}