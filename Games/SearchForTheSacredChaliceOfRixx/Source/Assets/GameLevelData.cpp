#include "GameLevelData.h"
#include "Assets/ZipLine.h"

GameLevelData::GameLevelData()
{
}

/*virtual*/ GameLevelData::~GameLevelData()
{
}

/*virtual*/ bool GameLevelData::Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache)
{
	if (!LevelData::Load(jsonDoc, assetCache))
		return false;

	if (jsonDoc.HasMember("zip_lines"))
	{
		const rapidjson::Value& zipLinesValue = jsonDoc["zip_lines"];
		if (!zipLinesValue.IsArray())
			return false;

		for (int i = 0; i < zipLinesValue.Size(); i++)
		{
			if (!zipLinesValue[i].IsString())
				return false;

			std::string zipLineFile = zipLinesValue[i].GetString();

			Imzadi::Reference<Imzadi::Asset> asset;
			if (!assetCache->LoadAsset(zipLineFile, asset))
				return false;

			Imzadi::Reference<ZipLine> zipLine;
			zipLine.SafeSet(asset.Get());
			if (!zipLine)
				return false;

			this->zipLineArray.push_back(zipLine);
		}
	}

	return true;
}

/*virtual*/ bool GameLevelData::Unload()
{
	LevelData::Unload();

	this->zipLineArray.clear();

	return true;
}