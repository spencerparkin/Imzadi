#pragma once

#include "Assets/LevelData.h"

class ZipLine;

class GameLevelData : public Imzadi::LevelData
{
public:
	GameLevelData();
	virtual ~GameLevelData();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Unload() override;

	const std::vector<Imzadi::Reference<ZipLine>>& GetZipLineArray() { return this->zipLineArray; }
	const std::vector<std::string>& GetCubieFilesArray() { return this->cubieFilesArray; }
	const std::vector<std::string>& GetDoorFilesArray() { return this->doorFilesArray; }

protected:
	std::vector<Imzadi::Reference<ZipLine>> zipLineArray;
	std::vector<std::string> cubieFilesArray;
	std::vector<std::string> doorFilesArray;
};