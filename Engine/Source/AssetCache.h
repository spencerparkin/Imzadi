#pragma once

#include "Reference.h"
#include "rapidjson/document.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include <string>
#include <unordered_map>
#include <filesystem>

namespace Imzadi
{
	class Game;
	class Asset;
	class RenderObject;

	/**
	 * This is supposed to be one-stop-shopping for any asset we'd want to load,
	 * whether that be a render-mesh, texture, shader, etc.
	 */
	class IMZADI_API AssetCache : public ReferenceCounted
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
		bool LoadAsset(const std::string& assetFile, Reference<Asset>& asset);

		/**
		 * Save the given asset at the given file location.  If the given asset reference
		 * is null, then we look in the cache for the asset, and the given asset reference
		 * is changed to point to the saved asset, if it was found in the cache.
		 *
		 * Note that if the file already exists on disk, it will be replaced.  Also note that
		 * no check is made to make sure that the extension on the given file name is correct.
		 *
		 * @param[in] assetFile This is a fully-qualified path to where the asset should exist on disk.
		 * @param[in] asset A reference to the asset to save.  This can be null to search the cache instead.
		 * @return True is returend if and only if the asset save succeeds.
		 */
		bool SaveAsset(const std::string& assetFile, Reference<Asset>& asset);

		/**
		 * Remove all assets from this cache.
		 */
		void Clear();

		/**
		 * Resolve the given path into a fully qualified path, if it isn't already such a path.
		 *
		 * @param[in, out] assetFile This is a string holding the path to resolve.
		 * @return True is returned if the path was successfully resolved; false, otherwise.
		 */
		bool ResolveAssetPath(std::string& assetFile);

		/**
		 * Add a location where assets can be found.
		 * 
		 * @param assetFolder This should be a fully-qualified path to a folder upon which relative paths can be resolved.
		 */
		void AddAssetFolder(const std::string& assetFolder);

	protected:

		std::string MakeKey(const std::string& assetFile);
		Asset* FindAsset(const std::string& assetFile, std::string* key = nullptr);
		
		/**
		 * This must be overridden for custom asset types.  Don't forget to
		 * call this base-class method.
		 */
		virtual Asset* CreateBlankAssetForFileType(const std::string& assetFile);

		std::vector<std::filesystem::path>* assetFolderArray;

		typedef std::unordered_map<std::string, Reference<Asset>> AssetMap;
		AssetMap assetMap;
	};

	/**
	 * This is the base class for all asset types.
	 */
	class IMZADI_API Asset : public ReferenceCounted
	{
	public:
		Asset();
		virtual ~Asset();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) = 0;
		virtual bool Unload() = 0;
		virtual bool Save(rapidjson::Document& jsonDoc) const;
		virtual bool CanBeCached() const { return true; }
		virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) { return false; }

		static bool LoadVector(const rapidjson::Value& vectorValue, Imzadi::Vector3& vector);
		static bool LoadEulerAngles(const rapidjson::Value& eulerAnglesValue, Imzadi::Quaternion& quat);
		static bool LoadQuaternion(const rapidjson::Value& quaternionValue, Imzadi::Quaternion& quat);
		static bool LoadStringArray(const rapidjson::Value& stringArrayValue, std::vector<std::string>& stringArray);
		static bool LoadBoundingBox(const rapidjson::Value& aabbValue, Imzadi::AxisAlignedBoundingBox& aabb);
		static bool LoadTransform(const rapidjson::Value& transformValue, Imzadi::Transform& transform);
		static bool LoadMatrix(const rapidjson::Value& matrixValue, Imzadi::Matrix3x3& matrix);

		static void SaveVector(rapidjson::Value& vectorValue, const Imzadi::Vector3& vector, rapidjson::Document* doc);
		static void SaveEulerAngles(rapidjson::Value& eulerAnglesValue, const Imzadi::Quaternion& quat, rapidjson::Document* doc);
		static void SaveQuaternion(rapidjson::Value& quaternionValue, const Imzadi::Quaternion& quat, rapidjson::Document* doc);
		static void SaveStringArray(rapidjson::Value& stringArrayValue, const std::vector<std::string>& stringArray, rapidjson::Document* doc);
		static void SaveBoundingBox(rapidjson::Value& aabbValue, const Imzadi::AxisAlignedBoundingBox& aabb, rapidjson::Document* doc);
		static void SaveTransform(rapidjson::Value& transformValue, const Imzadi::Transform& transform, rapidjson::Document* doc);
		static void SaveMatrix(rapidjson::Value& matrixValue, const Imzadi::Matrix3x3& matrix, rapidjson::Document* doc);
	};
}