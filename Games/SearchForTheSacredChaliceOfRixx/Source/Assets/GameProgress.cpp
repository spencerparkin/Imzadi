#include "GameProgress.h"

GameProgress::GameProgress()
{
	this->levelName = "Level1";
	this->numLives = 3;
}

/*virtual*/ GameProgress::~GameProgress()
{
}

/*virtual*/ bool GameProgress::Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache)
{
	if (jsonDoc.HasMember("level_name") && jsonDoc["level_name"].IsString())
		this->levelName = jsonDoc["level_name"].GetString();
	else
		this->levelName = "Level1";

	if (jsonDoc.HasMember("num_lives") && jsonDoc["num_lives"].IsInt())
		this->numLives = jsonDoc["num_lives"].GetInt();
	else
		this->numLives = 3;

	this->mileStoneSet.clear();
	this->inventoryMap.clear();

	if (this->numLives > 0)
	{
		if (jsonDoc.HasMember("mile_stone_array") && jsonDoc["mile_stone_array"].IsArray())
		{
			const rapidjson::Value& mileStoneArrayValue = jsonDoc["mile_stone_array"];
			for (int i = 0; i < mileStoneArrayValue.Size(); i++)
			{
				const rapidjson::Value& mileStoneValue = mileStoneArrayValue[i];
				if (!mileStoneValue.IsString())
					return false;

				this->mileStoneSet.insert(mileStoneValue.GetString());
			}
		}

		if (jsonDoc.HasMember("inventory_map") && jsonDoc["inventory_map"].IsObject())
		{
			const rapidjson::Value& inventoryMapValue = jsonDoc["inventory_map"];
			for (auto iter = inventoryMapValue.MemberBegin(); iter != inventoryMapValue.MemberEnd(); iter++)
			{
				if (!iter->value.IsUint())
					return false;

				std::string itemName = iter->name.GetString();
				uint32_t itemCount = iter->value.GetUint();
				this->SetPossessedItemCount(itemName, itemCount);
			}
		}
	}
	else
	{
		this->numLives = 3;
		this->levelName = "Level1";
	}

	return true;
}

/*virtual*/ bool GameProgress::Save(rapidjson::Document& jsonDoc) const
{
	jsonDoc.SetObject();
	jsonDoc.AddMember("level_name", rapidjson::Value().SetString(this->levelName.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("num_lives", rapidjson::Value().SetInt(this->numLives), jsonDoc.GetAllocator());

	rapidjson::Value mileStoneArrayValue;
	mileStoneArrayValue.SetArray();
	for (const std::string& mileStone : this->mileStoneSet)
		mileStoneArrayValue.PushBack(rapidjson::Value().SetString(mileStone.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());

	jsonDoc.AddMember("mile_stone_array", mileStoneArrayValue, jsonDoc.GetAllocator());

	rapidjson::Value inventoryMapValue;
	inventoryMapValue.SetObject();
	for (const auto& pair : this->inventoryMap)
		inventoryMapValue.AddMember(rapidjson::Value().SetString(pair.first.c_str(), jsonDoc.GetAllocator()), rapidjson::Value().SetUint(pair.second), jsonDoc.GetAllocator());

	jsonDoc.AddMember("inventory_map", inventoryMapValue, jsonDoc.GetAllocator());

	return true;
}

/*virtual*/ bool GameProgress::Unload()
{
	this->levelName = "";
	this->inventoryMap.clear();
	this->mileStoneSet.clear();
	return true;
}

bool GameProgress::WasMileStoneReached(const std::string& mileStone) const
{
	return this->mileStoneSet.find(mileStone) != this->mileStoneSet.end();
}

void GameProgress::SetMileStoneReached(const std::string& mileStone)
{
	this->mileStoneSet.insert(mileStone);
}

uint32_t GameProgress::GetPossessedItemCount(const std::string& itemName) const
{
	std::unordered_map<std::string, uint32_t>::const_iterator iter = this->inventoryMap.find(itemName);
	if (iter == this->inventoryMap.end())
		return 0;

	return iter->second;
}

void GameProgress::SetPossessedItemCount(const std::string& itemName, uint32_t itemCount)
{
	std::unordered_map<std::string, uint32_t>::iterator iter = this->inventoryMap.find(itemName);
	if (iter != this->inventoryMap.end())
		this->inventoryMap.erase(iter);

	if (itemCount > 0)
		this->inventoryMap.insert(std::pair<std::string, uint32_t>(itemName, itemCount));
}

void GameProgress::SetLevelName(const std::string& levelName)
{
	this->levelName = levelName;
}

const std::string& GameProgress::GetLevelName() const
{
	return this->levelName;
}

void GameProgress::SetNumLives(int numLives)
{
	this->numLives = numLives;
}

int GameProgress::GetNumLives() const
{
	return this->numLives;
}