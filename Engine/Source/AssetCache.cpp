#include "AssetCache.h"
#include "Assets/RenderMesh.h"
#include "Assets/Buffer.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Assets/CollisionShapeSet.h"
#include "Assets/LevelData.h"
#include "Assets/MovingPlatformData.h"
#include "Assets/Animation.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/SkinWeights.h"
#include "Assets/Skeleton.h"
#include "Log.h"
#include "Game.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"

using namespace Imzadi;

//-------------------------------- AssetCache --------------------------------

AssetCache::AssetCache()
{
}

/*virtual*/ AssetCache::~AssetCache()
{
}

/*virtual*/ void AssetCache::Clear()
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
	if (assetFile.length() == 0)
		return false;

	std::filesystem::path assetPath(assetFile);
	if (assetPath.is_absolute())
		return std::filesystem::exists(assetPath);

	for (const std::filesystem::path& assetFolder : this->assetFolderArray)
	{		
		std::string fullyQualifiedPath = (assetFolder / assetPath).string();
		if (std::filesystem::exists(fullyQualifiedPath))
		{
			assetFile = fullyQualifiedPath;
			return true;
		}
	}

	return false;
}

void AssetCache::AddAssetFolder(const std::string& assetFolder)
{
	this->assetFolderArray.push_back(std::filesystem::path(assetFolder));
}

void AssetCache::RemoveAssetFolder(const std::string& assetFolder)
{
	std::filesystem::path givenFolder(assetFolder);

	for (int i = 0; i < (signed)this->assetFolderArray.size(); i++)
	{
		std::filesystem::path& existingFolder = this->assetFolderArray[i];

		if (existingFolder == assetFolder)
		{
			if (i < (signed)this->assetFolderArray.size() - 1)
				this->assetFolderArray[i] = this->assetFolderArray[this->assetFolderArray.size() - 1];
			
			this->assetFolderArray.pop_back();
			return;
		}
	}
}

std::string AssetCache::MakeKey(const std::string& assetFile)
{
	// TODO: Of course, we'll have a problem here if two files have the same name but are in different directories.
	std::filesystem::path assetPath(assetFile);
	std::string key = assetPath.filename().string();
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return std::tolower(c); });
	return key;
}

/*virtual*/ Asset* AssetCache::FindAsset(const std::string& assetFile, std::string* key /*= nullptr*/)
{
	std::string keyStorage;
	if (!key)
		key = &keyStorage;
	*key = this->MakeKey(assetFile);
	AssetMap::iterator iter = this->assetMap.find(*key);
	if (iter == this->assetMap.end())
		return nullptr;
	return iter->second;
}

/*virtual*/ Asset* AssetCache::CreateBlankAssetForFileType(const std::string& assetFile)
{
	std::filesystem::path assetPath(assetFile);
	std::string ext = assetPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
	if (ext == ".render_mesh")
		return new RenderMeshAsset();
	else if (ext == ".shader")
		return new Shader();
	else if (ext == ".texture")
		return new Texture();
	else if (ext == ".buffer")
		return new Buffer();
	else if (ext == ".collision")
		return new CollisionShapeSet();
	else if (ext == ".level")
		return new LevelData();
	else if (ext == ".mov_plat")
		return new MovingPlatformData();
	else if (ext == ".skinned_render_mesh")
		return new SkinnedRenderMesh();
	else if (ext == ".animation")
		return new Animation();
	else if (ext == ".skin_weights")
		return new SkinWeights();
	else if (ext == ".skeleton")
		return new Skeleton();

	return nullptr;
}

bool AssetCache::LoadAsset(const std::string& assetFile, Reference<Asset>& asset)
{
	std::string key;
	asset.Set(this->FindAsset(assetFile, &key));
	if (asset)
		return true;

	std::string resolvedAssetFile(assetFile);
	if (!ResolveAssetPath(resolvedAssetFile))
	{
		IMZADI_LOG_ERROR("Failed to resolve path: " + assetFile);
		return false;
	}

	asset.Set(this->CreateBlankAssetForFileType(assetFile));
	if (!asset)
	{
		IMZADI_LOG_ERROR("Failed to create blank asset type for file: " + assetFile);
		return false;
	}

	std::ifstream fileStream;
	fileStream.open(resolvedAssetFile, std::ios::in);
	if (!fileStream.is_open())
	{
		IMZADI_LOG_ERROR("Failed to open (for reading) the file: " + assetFile);
		return false;
	}

	rapidjson::IStreamWrapper streamWrapper(fileStream);
	rapidjson::Document jsonDoc;
	jsonDoc.ParseStream(streamWrapper);

	if (jsonDoc.HasParseError())
	{
		// TODO: It would be nice if we could get line and column numbers in the error message here.
		asset.Reset();
		rapidjson::ParseErrorCode errorCode = jsonDoc.GetParseError();
		IMZADI_LOG_ERROR(rapidjson::GetParseError_En(errorCode));
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

	return true;
}

bool AssetCache::SaveAsset(const std::string& assetFile, Reference<Asset>& asset)
{
	if (!asset)
	{
		asset.Set(this->FindAsset(assetFile));
		if (!asset)
		{
			IMZADI_LOG_ERROR("No asset to save.");
			return false;
		}
	}

	rapidjson::Document doc;
	if (!asset->Save(doc))
		return false;

	if (std::filesystem::exists(assetFile))
		std::remove(assetFile.c_str());

	std::ofstream fileStream;
	fileStream.open(assetFile, std::ios::out);
	if (!fileStream.is_open())
	{
		IMZADI_LOG_ERROR("Failed to open (for writing) the file: " + assetFile);
		return false;
	}

	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(stringBuffer);
	if (!doc.Accept(prettyWriter))
	{
		IMZADI_LOG_ERROR("Failed to generate JSON text from JSON data.");
		return false;
	}

	fileStream << stringBuffer.GetString();
	fileStream.close();
	return true;
}

//-------------------------------- Asset --------------------------------

Asset::Asset()
{
}

/*virtual*/ Asset::~Asset()
{
}

/*virtual*/ bool Asset::Save(rapidjson::Document& jsonDoc) const
{
	IMZADI_LOG_ERROR("Save no implimented.");
	return false;
}

/*static*/ bool Asset::LoadVector(const rapidjson::Value& vectorValue, Vector3& vector)
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

/*static*/ bool Asset::LoadEulerAngles(const rapidjson::Value& eulerAnglesValue, Quaternion& quat)
{
	if (!eulerAnglesValue.IsObject())
		return false;

	if (!eulerAnglesValue.HasMember("yaw") || !eulerAnglesValue.HasMember("pitch") || !eulerAnglesValue.HasMember("roll"))
		return false;

	if (!eulerAnglesValue["yaw"].IsFloat() || !eulerAnglesValue["pitch"].IsFloat() || !eulerAnglesValue["roll"].IsFloat())
		return false;

	double yawAngle = IMZADI_DEGS_TO_RADS(eulerAnglesValue["yaw"].GetFloat());
	double pitchAngle = IMZADI_DEGS_TO_RADS(eulerAnglesValue["pitch"].GetFloat());
	double rollAngle = IMZADI_DEGS_TO_RADS(eulerAnglesValue["roll"].GetFloat());

	Quaternion yawQuat, pitchQuat, rollQuat;

	yawQuat.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), yawAngle);
	pitchQuat.SetFromAxisAngle(Vector3(1.0, 0.0, 0.0), pitchAngle);
	rollQuat.SetFromAxisAngle(Vector3(0.0, 0.0, 1.0), rollAngle);

	quat = yawQuat * pitchQuat * rollQuat;
	quat = quat.Normalized();	// Do this just to account for any round-off error.

	return true;
}

/*static*/ bool Asset::LoadQuaternion(const rapidjson::Value& quaternionValue, Quaternion& quat)
{
	if (!quaternionValue.IsObject())
		return false;

	if (!quaternionValue.HasMember("x") || !quaternionValue.HasMember("y") || !quaternionValue.HasMember("z") || !quaternionValue.HasMember("w"))
		return false;

	if (!quaternionValue["x"].IsFloat() || !quaternionValue["y"].IsFloat() || !quaternionValue["z"].IsFloat() || !quaternionValue["w"].IsFloat())
		return false;

	quat.x = quaternionValue["x"].GetFloat();
	quat.y = quaternionValue["y"].GetFloat();
	quat.z = quaternionValue["z"].GetFloat();
	quat.w = quaternionValue["w"].GetFloat();
	return true;
}

/*static*/ bool Asset::LoadStringArray(const rapidjson::Value& stringArrayValue, std::vector<std::string>& stringArray)
{
	if (!stringArrayValue.IsArray())
		return false;

	stringArray.clear();

	for (int i = 0; i < stringArrayValue.Size(); i++)
	{
		const rapidjson::Value& stringValue = stringArrayValue[i];
		if (!stringValue.IsString())
			return false;

		stringArray.push_back(stringValue.GetString());
	}

	return true;
}

/*static*/ bool Asset::LoadBoundingBox(const rapidjson::Value& aabbValue, AxisAlignedBoundingBox& aabb)
{
	if (!aabbValue.IsObject())
		return false;

	if (!aabbValue.HasMember("min") || !aabbValue.HasMember("max"))
		return false;

	if (!LoadVector(aabbValue["min"], aabb.minCorner))
		return false;

	if (!LoadVector(aabbValue["max"], aabb.maxCorner))
		return false;

	return true;
}

/*static*/ bool Asset::LoadTransform(const rapidjson::Value& transformValue, Transform& transform)
{
	if (!transformValue.IsObject())
		return false;

	if (!transformValue.HasMember("matrix") || !transformValue.HasMember("translation"))
		return false;

	if (!LoadMatrix(transformValue["matrix"], transform.matrix))
		return false;

	if (!LoadVector(transformValue["translation"], transform.translation))
		return false;

	return true;
}

/*static*/ bool Asset::LoadMatrix(const rapidjson::Value& matrixValue, Matrix3x3& matrix)
{
	if (!matrixValue.IsArray() || matrixValue.Size() != 9)
		return false;

	for (int i = 0; i < 9; i++)
	{
		const rapidjson::Value& elementValue = matrixValue[i];
		if (!elementValue.IsFloat())
			return false;

		int row = i / 3;
		int col = i % 3;
		matrix.ele[row][col] = elementValue.GetFloat();
	}

	return true;
}

/*static*/ bool Asset::LoadAnimTransform(const rapidjson::Value& transformValue, AnimTransform& transform)
{
	if (!transformValue.IsObject())
		return false;

	if (!transformValue.HasMember("scale") || !transformValue.HasMember("rotation") || !transformValue.HasMember("translation"))
		return false;

	if (!LoadVector(transformValue["scale"], transform.scale))
		return false;

	if (!LoadQuaternion(transformValue["rotation"], transform.rotation))
		return false;

	if (!LoadVector(transformValue["translation"], transform.translation))
		return false;

	return true;
}

/*static*/ void Asset::SaveVector(rapidjson::Value& vectorValue, const Vector3& vector, rapidjson::Document* doc)
{
	vectorValue.SetObject();
	vectorValue.AddMember("x", rapidjson::Value().SetFloat(vector.x), doc->GetAllocator());
	vectorValue.AddMember("y", rapidjson::Value().SetFloat(vector.y), doc->GetAllocator());
	vectorValue.AddMember("z", rapidjson::Value().SetFloat(vector.z), doc->GetAllocator());
}

/*static*/ void Asset::SaveEulerAngles(rapidjson::Value& eulerAnglesValue, const Quaternion& quat, rapidjson::Document* doc)
{
	eulerAnglesValue.SetObject();

	// TODO: This requires a factorization!
	assert(false);
}

/*static*/ void Asset::SaveQuaternion(rapidjson::Value& quaternionValue, const Quaternion& quat, rapidjson::Document* doc)
{
	quaternionValue.SetObject();
	quaternionValue.AddMember("x", rapidjson::Value().SetFloat(quat.x), doc->GetAllocator());
	quaternionValue.AddMember("y", rapidjson::Value().SetFloat(quat.y), doc->GetAllocator());
	quaternionValue.AddMember("z", rapidjson::Value().SetFloat(quat.z), doc->GetAllocator());
	quaternionValue.AddMember("w", rapidjson::Value().SetFloat(quat.w), doc->GetAllocator());
}

/*static*/ void Asset::SaveStringArray(rapidjson::Value& stringArrayValue, const std::vector<std::string>& stringArray, rapidjson::Document* doc)
{
	stringArrayValue.SetArray();

	for (const std::string& str : stringArray)
		stringArrayValue.PushBack(rapidjson::Value().SetString(str.c_str(), doc->GetAllocator()), doc->GetAllocator());
}

/*static*/ void Asset::SaveBoundingBox(rapidjson::Value& aabbValue, const AxisAlignedBoundingBox& aabb, rapidjson::Document* doc)
{
	rapidjson::Value minValue, maxValue;

	SaveVector(minValue, aabb.minCorner, doc);
	SaveVector(maxValue, aabb.maxCorner, doc);

	aabbValue.SetObject();
	aabbValue.AddMember("min", minValue, doc->GetAllocator());
	aabbValue.AddMember("max", maxValue, doc->GetAllocator());
}

/*static*/ void Asset::SaveTransform(rapidjson::Value& transformValue, const Transform& transform, rapidjson::Document* doc)
{
	rapidjson::Value matrixValue, translationValue;

	SaveMatrix(matrixValue, transform.matrix, doc);
	SaveVector(translationValue, transform.translation, doc);

	transformValue.SetObject();
	transformValue.AddMember("matrix", matrixValue, doc->GetAllocator());
	transformValue.AddMember("translation", translationValue, doc->GetAllocator());
}

/*static*/ void Asset::SaveMatrix(rapidjson::Value& matrixValue, const Matrix3x3& matrix, rapidjson::Document* doc)
{
	matrixValue.SetArray();

	for (int i = 0; i < 9; i++)
	{
		int row = i / 3;
		int col = i % 3;

		matrixValue.PushBack(rapidjson::Value().SetFloat(matrix.ele[row][col]), doc->GetAllocator());
	}
}

/*static*/ void Asset::SaveAnimTransform(rapidjson::Value& transformValue, const AnimTransform& transform, rapidjson::Document* doc)
{
	rapidjson::Value scaleValue, rotationValue, translationValue;

	SaveVector(scaleValue, transform.scale, doc);
	SaveQuaternion(rotationValue, transform.rotation, doc);
	SaveVector(translationValue, transform.translation, doc);

	transformValue.SetObject();
	transformValue.AddMember("scale", scaleValue, doc->GetAllocator());
	transformValue.AddMember("rotation", rotationValue, doc->GetAllocator());
	transformValue.AddMember("translation", translationValue, doc->GetAllocator());
}