#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"

/**
 * These are used to describe everything that goes into a level
 * and possibly how the level operates in terms of animated platforms,
 * chests, player-start positions, keys, locked doors, or anything else
 * like that.  It also describes where all the bad guys are and what
 * kind, etc.
 * 
 * For now, at a minimum, it contains info about what models and static
 * collision files to load.
 */
class LevelData : public Asset
{
public:
	LevelData();
	virtual ~LevelData();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;

	std::vector<std::string> modelFilesArray;
	std::vector<std::string> collisionFilesArray;
	Collision::Vector3 playerStartPosition;
	Collision::Quaternion playerStartOrientation;
};