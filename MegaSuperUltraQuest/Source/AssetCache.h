#pragma once

#include "Reference.h"
#include "rapidjson/document.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/AxisAlignedBoundingBox.h"
#include <string>
#include <unordered_map>
#include <filesystem>

class Game;
class Asset;
class RenderObject;

/**
 * This is supposed to be one-stop-shopping for any asset we'd want to load,
 * whether that be a render-mesh, texture, shader, etc.
 */
class AssetCache : public ReferenceCounted
{
public:
	AssetCache();
	virtual ~AssetCache();

	/**
	 * Load the asset at the given file location, hitting the cache if possible.
	 * On cache miss, the cache is updated if the asset allows it.
	 * 
	 * @param[in] assetFile This is a relative or fully-qualified path to where the asset exists on disk.
	 * @param[out] asset A reference to the asset is returned here.
	 * @return True is returned if and only if the asset grab succeeded.  On failure, the given reference should be null.
	 */
	bool GrabAsset(const std::string& assetFile, Reference<Asset>& asset);

	/**
	 * Remove all assets from this cache.
	 */
	void Clear();

	/**
	 * Resolve the given path into a fully qualified path, if it isn't already such a path.
	 * 
	 * @param[in, out] assetFile This is a string holding the path to resolve.
	 */
	bool ResolveAssetPath(std::string& assetFile);

	void SetAssetFolder(const std::string& assetFolder) { this->assetFolder = assetFolder; }

	std::string GetAssetFolder() const { return this->assetFolder.string(); }

private:

	std::filesystem::path assetFolder;

	typedef std::unordered_map<std::string, Reference<Asset>> AssetMap;
	AssetMap assetMap;
};

/**
 * This is the base class for all asset types.
 */
class Asset : public ReferenceCounted
{
public:
	Asset();
	virtual ~Asset();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) = 0;
	virtual bool Unload() = 0;
	virtual bool Save(rapidjson::Document& jsonDoc) const;
	virtual bool CanBeCached() const { return true; }
	virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) { return false; }

protected:
	bool LoadVector(const rapidjson::Value& vectorValue, Collision::Vector3& vector);
	bool LoadEulerAngles(const rapidjson::Value& eulerAnglesValue, Collision::Quaternion& quat);
	bool LoadStringArray(const rapidjson::Value& stringArrayValue, std::vector<std::string>& stringArray);
	bool LoadBoundingBox(const rapidjson::Value& aabbValue, Collision::AxisAlignedBoundingBox& aabb);
};