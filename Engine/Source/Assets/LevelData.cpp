#include "LevelData.h"
#include "Error.h"

using namespace Imzadi;

LevelData::LevelData()
{
}

/*virtual*/ LevelData::~LevelData()
{
}

/*virtual*/ bool LevelData::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_ERROR("The given JSON doc was not an object.");
		return false;
	}

	if (!jsonDoc.HasMember("static_meshes"))
	{
		IMZADI_ERROR("No \"static_meshes\" member found.");
		return false;
	}

	if (!this->LoadStringArray(jsonDoc["static_meshes"], this->modelFilesArray))
	{
		IMZADI_ERROR("Failed to load \"static_meshes\" as an array of strings.");
		return false;
	}

	if (!jsonDoc.HasMember("static_collision"))
	{
		IMZADI_ERROR("No \"static_collision\" member found.");
		return false;
	}

	if (!this->LoadStringArray(jsonDoc["static_collision"], this->collisionFilesArray))
	{
		IMZADI_ERROR("Failed to load \"static_collision\" member as array of strings.");
		return false;
	}

	this->playerStartPosition.SetComponents(0.0, 0.0, 0.0);
	if (jsonDoc.HasMember("player_start_location") && !this->LoadVector(jsonDoc["player_start_location"], this->playerStartPosition))
	{
		IMZADI_ERROR("No \"player_start_location\" member found or it did not load properly as a vector.");
		return false;
	}

	this->playerStartOrientation.SetIdentity();
	if (jsonDoc.HasMember("player_start_orientation") && !this->LoadEulerAngles(jsonDoc["player_start_orientation"], this->playerStartOrientation))
	{
		IMZADI_ERROR("No \"player_start_orientation\" member found or it did notload properly as an orientation.");
		return false;
	}

	this->movingPlatformFilesArray.clear();
	if (jsonDoc.HasMember("moving_platforms") && !this->LoadStringArray(jsonDoc["moving_platforms"], this->movingPlatformFilesArray))
	{
		IMZADI_ERROR("No \"moving_platforms\" member found or it did not load properly.");
		return false;
	}

	return true;
}

/*virtual*/ bool LevelData::Unload()
{
	this->modelFilesArray.clear();
	this->collisionFilesArray.clear();
	this->movingPlatformFilesArray.clear();

	return true;
}