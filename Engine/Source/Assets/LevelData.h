#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"

namespace Imzadi
{
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
	class IMZADI_API LevelData : public Asset
	{
	public:
		LevelData();
		virtual ~LevelData();

		virtual bool Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache) override;
		virtual bool Unload() override;

		const std::vector<std::string>& GetModelFilesArray() { return this->modelFilesArray; }
		const std::vector<std::string>& GetCollisionFilesArray() { return this->collisionFilesArray; }
		const std::vector<std::string>& GetMovingPlatformFilesArray() { return this->movingPlatformFilesArray; }
		const Vector3& GetPlayerStartPosition() { return this->playerStartPosition; }
		const Quaternion& GetPlayerStartOrientation() { return this->playerStartOrientation; }

	private:
		std::vector<std::string> modelFilesArray;
		std::vector<std::string> collisionFilesArray;
		std::vector<std::string> movingPlatformFilesArray;
		Vector3 playerStartPosition;
		Quaternion playerStartOrientation;
	};
}