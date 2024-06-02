#include "AssetCache.h"
#include "Assets/RenderMesh.h"
#include "Assets/Buffer.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/LevelData.h"
#include "Game.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"

using namespace Collision;

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
		Asset* asset = pair.second.Get();
		asset->Unload();
	}

	this->assetMap.clear();
}

bool AssetCache::ResolveAssetPath(std::string& assetFile)
{
	std::filesystem::path assetPath(assetFile);
	if (assetPath.is_relative())
		assetFile = (this->assetFolder / assetPath).string();

	return std::filesystem::exists(assetFile);
}

bool AssetCache::GrabAsset(const std::string& assetFile, Reference<Asset>& asset)
{
	// Of course, we'll have a problem here if two files have the same name but are in different directories.
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
	asset.Reset();
	if (ext == ".render_mesh")
		asset.Set(new RenderMeshAsset());
	else if (ext == ".shader")
		asset.Set(new Shader());
	else if (ext == ".texture")
		asset.Set(new Texture());
	else if (ext == ".buffer")
		asset.Set(new Buffer());
	else if (ext == ".collision")
		asset.Set(new CollisionShapeSet());
	else if (ext == ".level")
		asset.Set(new LevelData());
	if (!asset)
		return false;

	// Try to load the asset.
	std::ifstream fileStream;
	fileStream.open(resolvedAssetFile, std::ios::in);
	if (!fileStream.is_open())
		return false;

	rapidjson::IStreamWrapper streamWrapper(fileStream);
	rapidjson::Document jsonDoc;
	jsonDoc.ParseStream(streamWrapper);

	if (jsonDoc.HasParseError())
	{
		// TODO: It would be nice if we could get line and column numbers in the error message here.
		asset.Reset();
		rapidjson::ParseErrorCode errorCode = jsonDoc.GetParseError();
		const char* errorMsg = rapidjson::GetParseError_En(errorCode);
		MessageBoxA(Game::Get()->GetMainWindowHandle(), errorMsg, "JSON parse error!", MB_ICONERROR | MB_OK);
		return false;
	}

	if (!asset->Load(jsonDoc, this))
	{
		asset.Reset();
		return false;
	}

	// Having loaded the asset successfully, cache it.  Ask the asset
	// if it can be cached before doing so.  I'm not sure, but there
	// may be a future need for this.
	if (asset->CanBeCached())
		this->assetMap.insert(std::pair<std::string, Reference<Asset>>(key, asset));

	// Enjoy your meal.
	return true;
}

bool AssetCache::LoadVector(const rapidjson::Value& vectorValue, Collision::Vector3& vector)
{
	if (!vectorValue.IsObject())
		return false;

	if (!vectorValue.HasMember("x") || !vectorValue.HasMember("y") || !vectorValue.HasMember("z"))
		return false;

	if (!vectorValue["x"].IsFloat() || !vectorValue["y"].IsFloat() || !vectorValue["z"].IsFloat())
		return false;

	vector.x = vectorValue["x"].GetFloat();
	vector.y = vectorValue["y"].GetFloat();
	vector.z = vectorValue["z"].GetFloat();
	return true;
}

bool AssetCache::LoadBoundingBox(const rapidjson::Value& aabbValue, Collision::AxisAlignedBoundingBox& aabb)
{
	if (!aabbValue.IsObject())
		return false;

	if (!aabbValue.HasMember("min") || !aabbValue.HasMember("max"))
		return false;

	if (!this->LoadVector(aabbValue["min"], aabb.minCorner))
		return false;

	if (!this->LoadVector(aabbValue["max"], aabb.maxCorner))
		return false;

	return true;
}

//-------------------------------- Asset --------------------------------

Asset::Asset()
{
}

/*virtual*/ Asset::~Asset()
{
}

bool Asset::LoadVector(const rapidjson::Value& vectorValue, Collision::Vector3& vector)
{
	if (!vectorValue.IsObject())
		return false;

	if (!vectorValue.HasMember("x") || !vectorValue.HasMember("y") || !vectorValue.HasMember("z"))
		return false;

	if (!vectorValue["x"].IsFloat() || !vectorValue["y"].IsFloat() || !vectorValue["z"].IsFloat())
		return false;

	vector.x = vectorValue["x"].GetFloat();
	vector.y = vectorValue["y"].GetFloat();
	vector.z = vectorValue["z"].GetFloat();
	return true;
}

bool Asset::LoadEulerAngles(const rapidjson::Value& eulerAnglesValue, Collision::Quaternion& quat)
{
	if (!eulerAnglesValue.IsObject())
		return false;

	if (!eulerAnglesValue.HasMember("yaw") || !eulerAnglesValue.HasMember("pitch") || !eulerAnglesValue.HasMember("roll"))
		return false;

	if (!eulerAnglesValue["yaw"].IsFloat() || !eulerAnglesValue["pitch"].IsFloat() || !eulerAnglesValue["roll"].IsFloat())
		return false;

	double yawAngle = COLL_SYS_DEGS_TO_RADS(eulerAnglesValue["yaw"].GetFloat());
	double pitchAngle = COLL_SYS_DEGS_TO_RADS(eulerAnglesValue["pitch"].GetFloat());
	double rollAngle = COLL_SYS_DEGS_TO_RADS(eulerAnglesValue["roll"].GetFloat());

	Quaternion yawQuat, pitchQuat, rollQuat;

	yawQuat.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), yawAngle);
	pitchQuat.SetFromAxisAngle(Vector3(1.0, 0.0, 0.0), pitchAngle);
	rollQuat.SetFromAxisAngle(Vector3(0.0, 0.0, 1.0), rollAngle);

	quat = yawQuat * pitchQuat * rollQuat;
	quat = quat.Normalized();	// Do this just to account for any round-off error.

	return true;
}