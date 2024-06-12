#include "LevelData.h"

using namespace Imzadi;

LevelData::LevelData()
{
}

/*virtual*/ LevelData::~LevelData()
{
}

/*virtual*/ bool LevelData::Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("static_meshes"))
		return false;

	if (!this->LoadStringArray(jsonDoc["static_meshes"], this->modelFilesArray))
		return false;

	if (!jsonDoc.HasMember("static_collision"))
		return false;

	if (!this->LoadStringArray(jsonDoc["static_collision"], this->collisionFilesArray))
		return false;

	this->playerStartPosition.SetComponents(0.0, 0.0, 0.0);
	if (jsonDoc.HasMember("player_start_location") && !this->LoadVector(jsonDoc["player_start_location"], this->playerStartPosition))
		return false;

	this->playerStartOrientation.SetIdentity();
	if (jsonDoc.HasMember("player_start_orientation") && !this->LoadEulerAngles(jsonDoc["player_start_orientation"], this->playerStartOrientation))
		return false;

	this->movingPlatformFilesArray.clear();
	if (jsonDoc.HasMember("moving_platforms") && !this->LoadStringArray(jsonDoc["moving_platforms"], this->movingPlatformFilesArray))
		return false;

	return true;
}

/*virtual*/ bool LevelData::Unload()
{
	this->modelFilesArray.clear();
	this->collisionFilesArray.clear();
	this->movingPlatformFilesArray.clear();

	return true;
}