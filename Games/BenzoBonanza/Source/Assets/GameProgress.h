#pragma once

#include "AssetCache.h"
#include <unordered_map>
#include <unordered_set>

/**
 * This is the player's save-game data.  I don't have a front-end or
 * any kind of menuing system, so progress just gets saved automatically
 * as the game is played.  This class will also serve as the player's
 * inventory.
 */
class GameProgress : public Imzadi::Asset
{
public:
	GameProgress();
	virtual ~GameProgress();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Save(rapidjson::Document& jsonDoc) const override;
	virtual bool Unload() override;

	bool WasMileStoneReached(const std::string& mileStone) const;
	void SetMileStoneReached(const std::string& mileStone);

	uint32_t GetPossessedItemCount(const std::string& itemName) const;
	void SetPossessedItemCount(const std::string& itemName, uint32_t itemCount);

	void SetLevelName(const std::string& levelName);
	const std::string& GetLevelName() const;

	void SetNumLives(int numLives);
	int GetNumLives() const;

	const std::unordered_map<std::string, uint32_t>& GetInventoryMap() const { return this->inventoryMap; }

	bool WasBenzoCollectedAt(const Imzadi::Vector3& location) const;
	void SetBenzoCollectedAt(const Imzadi::Vector3& location);

	void CalcBenzoStats(int& totalNumBenzos, int& numBenzosCollected);

private:
	std::string MakeBenzoKey(const Imzadi::Vector3& location) const;
	int CountBenzosInLevelFile(const std::string& levelFile);

private:
	std::unordered_map<std::string, uint32_t> inventoryMap;
	std::unordered_set<std::string> mileStoneSet;
	std::unordered_set<std::string> benzoSet;
	std::string levelName;
	int numLives;
};