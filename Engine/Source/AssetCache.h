#pragma once

#include "Reference.h"
#include "rapidjson/document.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Math/AnimTransform.h"
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
		 * @param[out] error A human-readable error message is returned here if something went wrong.
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
		 * @param[out] error A human-readable error message is returned here if something went wrong.
		 * @return True is returend if and only if the asset save succeeds.
		 */
		bool SaveAsset(const std::string& assetFile, Reference<Asset>& asset);

		/**
		 * Remove all assets from this cache.
		 */
		virtual void Clear();

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

		/**
		 * Remove a folder from consideration of where an asset can be found.
		 * 
		 * @param assetFolder This should be a fully-qualified path to a folder upon which relative paths can be resolved.
		 */
		void RemoveAssetFolder(const std::string& assetFolder);

	protected:

		std::string MakeKey(const std::string& assetFile);
		
		/**
		 * Override this to provide a custom caching mechanism.
		 * 
		 * @param[in] assetFile This is typically a relative path to the desired asset.
		 * @param[out] key This is an optional parameter that, if given, is set to the cache key that was used.
		 * @return A pointer to a heap-allocated asset should be returned, if found; null, otherwise.  Either case is not an error unless the error parameter is filled out.
		 */
		virtual Asset* FindAsset(const std::string& assetFile, std::string* key = nullptr);
		
		/**
		 * This must be overridden for custom asset types.  Don't forget to
		 * call this base-class method in your override.
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

		/**
		 * Provide an asset loading implimentation here.
		 * 
		 * @param[in] jsonDoc This is JSON data from which the asset can be loaded.
		 * @param[in] assetCache For convenience, the asset cache is added to the loader, possibly for loading other assets recursively.
		 * @return True should be returned on success; false, otherwise.
		 */
		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) = 0;

		/**
		 * This is where any resources directly owned by the asset should be reclaimed.
		 * If the asset indirectly owns resources, those need not be cleaned up here.
		 * 
		 * @return True should be returned on success; false, otherwise.
		 */
		virtual bool Unload() = 0;

		/**
		 * Provide an asset saving implimentation here.  This method needs not be
		 * supported by all asset type.  The engine is primarily concerned with
		 * loading assets for run-time purposes, not saving them out.  However, in
		 * some cases, it is useful to provide the riciprical support of saving the
		 * asset to disk.  Since some assets load into GPU read-only memory, this is
		 * not always practical or possible.
		 * 
		 * @return True should be returned on success; false, otherwise.
		 */
		virtual bool Save(rapidjson::Document& jsonDoc) const;

		/**
		 * If the asset can be shared across owners, then true should be returned here.
		 * Some assets, though, may need to be instanced every time they're loaded, and
		 * so in such cases, false should be returned here.
		 */
		virtual bool CanBeCached() const { return true; }

		/**
		 * For asset types that are meant to be instanced into renderable objects,
		 * this method can be overridden to produce such an instance.
		 * 
		 * @param[out] renderObject On success, this should be set to a valid render objects; undefined, otherwise.
		 * @return Return true if a render object is successfully created; false, otherwise.
		 */
		virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) { return false; }

		static bool LoadVector(const rapidjson::Value& vectorValue, Vector3& vector);
		static bool LoadEulerAngles(const rapidjson::Value& eulerAnglesValue, Quaternion& quat);
		static bool LoadQuaternion(const rapidjson::Value& quaternionValue, Quaternion& quat);
		static bool LoadStringArray(const rapidjson::Value& stringArrayValue, std::vector<std::string>& stringArray);
		static bool LoadBoundingBox(const rapidjson::Value& aabbValue, AxisAlignedBoundingBox& aabb);
		static bool LoadTransform(const rapidjson::Value& transformValue, Transform& transform);
		static bool LoadMatrix(const rapidjson::Value& matrixValue, Matrix3x3& matrix);
		static bool LoadAnimTransform(const rapidjson::Value& transformValue, AnimTransform& transform);

		static void SaveVector(rapidjson::Value& vectorValue, const Vector3& vector, rapidjson::Document* doc);
		static void SaveEulerAngles(rapidjson::Value& eulerAnglesValue, const Quaternion& quat, rapidjson::Document* doc);
		static void SaveQuaternion(rapidjson::Value& quaternionValue, const Quaternion& quat, rapidjson::Document* doc);
		static void SaveStringArray(rapidjson::Value& stringArrayValue, const std::vector<std::string>& stringArray, rapidjson::Document* doc);
		static void SaveBoundingBox(rapidjson::Value& aabbValue, const AxisAlignedBoundingBox& aabb, rapidjson::Document* doc);
		static void SaveTransform(rapidjson::Value& transformValue, const Transform& transform, rapidjson::Document* doc);
		static void SaveMatrix(rapidjson::Value& matrixValue, const Matrix3x3& matrix, rapidjson::Document* doc);
		static void SaveAnimTransform(rapidjson::Value& transformValue, const AnimTransform& transform, rapidjson::Document* doc);
	};
}