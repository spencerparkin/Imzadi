#include "LevelData.h"
#include "Log.h"

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
		IMZADI_LOG_ERROR("The given JSON doc was not an object.");
		return false;
	}

	if (!jsonDoc.HasMember("static_meshes"))
	{
		IMZADI_LOG_ERROR("No \"static_meshes\" member found.");
		return false;
	}

	if (!this->LoadStringArray(jsonDoc["static_meshes"], this->modelFilesArray))
	{
		IMZADI_LOG_ERROR("Failed to load \"static_meshes\" as an array of strings.");
		return false;
	}

	if (!jsonDoc.HasMember("static_collision"))
	{
		IMZADI_LOG_ERROR("No \"static_collision\" member found.");
		return false;
	}

	if (!this->LoadStringArray(jsonDoc["static_collision"], this->collisionFilesArray))
	{
		IMZADI_LOG_ERROR("Failed to load \"static_collision\" member as array of strings.");
		return false;
	}

	this->playerStartPosition.SetComponents(0.0, 0.0, 0.0);
	if (jsonDoc.HasMember("player_start_location") && !this->LoadVector(jsonDoc["player_start_location"], this->playerStartPosition))
	{
		IMZADI_LOG_ERROR("The \"player_start_location\" member did not load.");
		return false;
	}

	this->playerStartOrientation.SetIdentity();
	if (jsonDoc.HasMember("player_start_orientation") && !this->LoadEulerAngles(jsonDoc["player_start_orientation"], this->playerStartOrientation))
	{
		IMZADI_LOG_ERROR("The \"player_start_orientation\" member did not load.");
		return false;
	}

	this->movingPlatformFilesArray.clear();
	if (jsonDoc.HasMember("moving_platforms") && !this->LoadStringArray(jsonDoc["moving_platforms"], this->movingPlatformFilesArray))
	{
		IMZADI_LOG_ERROR("The \"moving_platforms\" member did not load.");
		return false;
	}

	if (jsonDoc.HasMember("sky_dome") && jsonDoc["sky_dome"].IsString())
		this->skyDomeFile = jsonDoc["sky_dome"].GetString();

	if (jsonDoc.HasMember("cube_texture") && jsonDoc["cube_texture"].IsString())
		this->cubeTextureFile = jsonDoc["cube_texture"].GetString();

	this->triggerBoxFilesArray.clear();
	if (jsonDoc.HasMember("trigger_boxes") && !this->LoadStringArray(jsonDoc["trigger_boxes"], this->triggerBoxFilesArray))
	{
		IMZADI_LOG_ERROR("The \"trigger_boxes\" member did not load.");
		return false;
	}

	this->npcArray.clear();
	if (jsonDoc.HasMember("npc_array") && jsonDoc["npc_array"].IsArray())
	{
		const rapidjson::Value& npcArrayValue = jsonDoc["npc_array"];
		for (int i = 0; i < npcArrayValue.Size(); i++)
		{
			const rapidjson::Value& npcValue = npcArrayValue[i];
			if (!npcValue.IsObject())
			{
				IMZADI_LOG_ERROR("Expected NPC entry to be an object.");
				return false;
			}

			std::unique_ptr<NPC> npc(new NPC());
			npc->startPosition.SetComponents(0.0, 0.0, 0.0);
			npc->startOrientation.SetIdentity();
			npc->type = "?";

			if (npcValue.HasMember("type") && npcValue["type"].IsString())
				npc->type = npcValue["type"].GetString();

			if (npcValue.HasMember("start_position") && !this->LoadVector(npcValue["start_position"], npc->startPosition))
				return false;

			if (npcValue.HasMember("start_orientation") && !this->LoadEulerAngles(npcValue["start_orientation"], npc->startOrientation))
				return false;

			if (npcValue.HasMember("config") && npcValue["config"].IsObject())
			{
				const rapidjson::Value& configValue = npcValue["config"];
				for (auto iter = configValue.MemberBegin(); iter != configValue.MemberEnd(); ++iter)
				{
					if (!iter->value.IsString())
						return false;

					std::string key = iter->name.GetString();
					std::string value = iter->value.GetString();
					npc->configMap.insert(std::pair(key, value));
				}
			}

			this->npcArray.push_back(npc.release());
		}
	}

	return true;
}

/*virtual*/ bool LevelData::Unload()
{
	for (NPC* npc : this->npcArray)
		delete npc;

	this->modelFilesArray.clear();
	this->collisionFilesArray.clear();
	this->movingPlatformFilesArray.clear();
	this->triggerBoxFilesArray.clear();
	this->npcArray.clear();

	return true;
}