#include "GameProgress.h"
#include "GameApp.h"
#include "rapidjson/reader.h"
#include "rapidjson/istreamwrapper.h"
#include <fstream>

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
	this->benzoSet.clear();

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

		if (jsonDoc.HasMember("benzo_collection") && jsonDoc["benzo_collection"].IsArray())
		{
			const rapidjson::Value& benzoCollectionArrayValue = jsonDoc["benzo_collection"];
			for (int i = 0; i < benzoCollectionArrayValue.Size(); i++)
			{
				const rapidjson::Value& benzoValue = benzoCollectionArrayValue[i];
				if (!benzoValue.IsString())
					return false;

				this->benzoSet.insert(benzoValue.GetString());
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

	rapidjson::Value benzoCollectionArrayValue;
	benzoCollectionArrayValue.SetArray();
	for (const std::string& benzoKey : this->benzoSet)
		benzoCollectionArrayValue.PushBack(rapidjson::Value().SetString(benzoKey.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());

	jsonDoc.AddMember("benzo_collection", benzoCollectionArrayValue, jsonDoc.GetAllocator());

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
	this->benzoSet.clear();
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

bool GameProgress::WasBenzoCollectedAt(const Imzadi::Vector3& location) const
{
	std::string key = this->MakeBenzoKey(location);
	return this->benzoSet.find(key) != this->benzoSet.end();
}

void GameProgress::SetBenzoCollectedAt(const Imzadi::Vector3& location)
{
	std::string key = this->MakeBenzoKey(location);
	this->benzoSet.insert(key);
}

std::string GameProgress::MakeBenzoKey(const Imzadi::Vector3& location) const
{
	return std::format("<{}, {}, {}>", location.x, location.y, location.z);
}

void GameProgress::CalcBenzoStats(int& totalNumBenzos, int& numBenzosReturned)
{
	totalNumBenzos = 0;
	numBenzosReturned = (int)this->benzoSet.size();

	for (const auto& pair : this->inventoryMap)
	{
		const std::string& itemName = pair.first;
		if (itemName == "Ativan" ||
			itemName == "Halcion" ||
			itemName == "Klonopin" ||
			itemName == "Librium" ||
			itemName == "Restoril" ||
			itemName == "Valium" ||
			itemName == "Xanax")
		{
			// If the benzo is in our possession, then it has not yet been returned.
			uint32_t itemCount = pair.second;
			if (itemCount > 0)
				numBenzosReturned--;
		}
	}

	for (const std::filesystem::path& assetFolderPath : Imzadi::Game::Get()->GetAssetCache()->GetAssetFolderArray())
	{
		std::filesystem::path levelsFolder = assetFolderPath / "Levels";
		if (std::filesystem::exists(levelsFolder))
		{
			for (const auto& entry : std::filesystem::directory_iterator(levelsFolder))
			{
				totalNumBenzos += this->CountBenzosInLevelFile(entry.path().string());
			}
		}
	}
}

int GameProgress::CountBenzosInLevelFile(const std::string& levelFile)
{
	int numBenzos = 0;

	std::ifstream fileStream;
	fileStream.open(levelFile, std::ios::in);
	if(fileStream.is_open())
	{
		rapidjson::Document jsonDoc;
		rapidjson::IStreamWrapper streamWrapper(fileStream);
		jsonDoc.ParseStream(streamWrapper);
		if (!jsonDoc.HasParseError())
		{
			if (jsonDoc.IsObject() && jsonDoc.HasMember("npc_array") && jsonDoc["npc_array"].IsArray())
			{
				const rapidjson::Value& npcArrayValue = jsonDoc["npc_array"];
				for (int i = 0; i < npcArrayValue.Size(); i++)
				{
					const rapidjson::Value& npcValue = npcArrayValue[i];
					if (npcValue.IsObject() && npcValue.HasMember("type") && npcValue["type"].IsString() && npcValue["type"].GetString() == "benzo")
						numBenzos++;
				}
			}
		}

		fileStream.close();
	}

	return numBenzos;
}