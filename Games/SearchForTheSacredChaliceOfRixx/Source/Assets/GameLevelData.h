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

protected:
	std::vector<Imzadi::Reference<ZipLine>> zipLineArray;
};