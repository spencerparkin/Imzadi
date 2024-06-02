#include "LevelData.h"

LevelData::LevelData()
{
}

/*virtual*/ LevelData::~LevelData()
{
}

/*virtual*/ bool LevelData::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("static_meshes") || !jsonDoc["static_meshes"].IsArray())
		return false;

	this->modelFilesArray.clear();
	const rapidjson::Value& staticMeshesArrayValue = jsonDoc["static_meshes"];
	for (int i = 0; i < staticMeshesArrayValue.Size(); i++)
	{
		const rapidjson::Value& staticMeshValue = staticMeshesArrayValue[i];
		if (!staticMeshValue.IsString())
			return false;

		this->modelFilesArray.push_back(staticMeshValue.GetString());
	}

	if (!jsonDoc.HasMember("static_collision") || !jsonDoc["static_collision"].IsArray())
		return false;

	this->collisionFilesArray.clear();
	const rapidjson::Value& collisionFilesArrayValue = jsonDoc["static_collision"];
	for (int i = 0; i < collisionFilesArrayValue.Size(); i++)
	{
		const rapidjson::Value& collisionFileValue = collisionFilesArrayValue[i];
		if (!collisionFileValue.IsString())
			return false;

		this->collisionFilesArray.push_back(collisionFileValue.GetString());
	}

	this->playerStartPosition.SetComponents(0.0, 0.0, 0.0);
	if (jsonDoc.HasMember("player_start_location") && !this->LoadVector(jsonDoc["player_start_location"], this->playerStartPosition))
		return false;

	this->playerStartOrientation.SetIdentity();
	if (jsonDoc.HasMember("player_start_orientation") && !this->LoadEulerAngles(jsonDoc["player_start_orientation"], this->playerStartOrientation))
		return false;

	return true;
}

/*virtual*/ bool LevelData::Unload()
{
	this->modelFilesArray.clear();
	this->collisionFilesArray.clear();

	return true;
}