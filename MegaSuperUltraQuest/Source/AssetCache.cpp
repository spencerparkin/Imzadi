#include "AssetCache.h"
#include "RenderMesh.h"
#include "Shader.h"
#include "Texture.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "rapidjson/reader.h"

//-------------------------------- AssetCache --------------------------------

AssetCache::AssetCache()
{
}

/*virtual*/ AssetCache::~AssetCache()
{
}

void AssetCache::Clear()
{
	for (auto pair : this->assetMap)
	{
		Asset* asset = pair.second.get();
		asset->Unload();
	}

	this->assetMap.clear();
}

bool AssetCache::ResolveAssetPath(std::string& assetFile)
{
	std::filesystem::path assetPath(assetFile);
	if (assetPath.is_relative())
	{
		if (this->assetsFolder.empty())
		{
			this->assetsFolder = std::filesystem::current_path() / "Assets";
		}

		assetFile = (this->assetsFolder / assetPath).string();
	}

	return std::filesystem::exists(assetFile);
}

bool AssetCache::GrabAsset(const std::string& assetFile, std::shared_ptr<Asset>& asset)
{
	std::filesystem::path assetPath(assetFile);
	std::string key = assetPath.filename().string();
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });

	// Asset already cached?
	AssetMap::iterator iter = this->assetMap.find(key);
	if (iter != this->assetMap.end())
	{
		// Yes.  Here you go.  Enjoy your meal.
		asset = iter->second;
		return true;
	}

	// No.  We have to go load it.  First, resolve the path.
	std::string resolvedAssetFile(assetFile);
	if (!ResolveAssetPath(resolvedAssetFile))
		return false;

	// Next, what kind of asset is it?  We'll deduce this from the extension.
	std::string ext = assetPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
	asset.reset();
	if (ext == ".render_mesh")
		asset = std::make_shared<RenderMesh>();
	else if (ext == ".shader")
		asset = std::make_shared<Shader>();
	else if (ext == ".texture")
		asset = std::make_shared<Texture>();
	if (!asset)
		return false;

	// Try to load the asset.
	std::ifstream fileStream;
	fileStream.open(resolvedAssetFile, std::ios::in);
	if (!fileStream.is_open())
		return false;

	std::stringstream stringStream;
	stringStream << fileStream.rdbuf();
	std::string jsonText = stringStream.str();
	fileStream.close();

	rapidjson::Document jsonDoc;
	jsonDoc.Parse(jsonText.c_str());

	if (jsonDoc.HasParseError())
	{
		asset.reset();
		rapidjson::ParseErrorCode errorCode = jsonDoc.GetParseError();
		//...
		return false;
	}

	if (!asset->Load(jsonDoc, this))
	{
		asset.reset();
		return false;
	}

	// Having loaded the asset successfully, cache it.
	this->assetMap.insert(std::pair<std::string, std::shared_ptr<Asset>>(key, asset));

	// Enjoy your meal.
	return true;
}

//-------------------------------- Asset --------------------------------

Asset::Asset()
{
}

/*virtual*/ Asset::~Asset()
{
}