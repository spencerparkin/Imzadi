#pragma once

#include "Entities/Level.h"

class GameLevel : public Imzadi::Level
{
public:
	GameLevel();
	virtual ~GameLevel();

	virtual bool Setup() override;
	virtual bool SetupWithLevelData(Imzadi::LevelData* levelData) override;
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;
	virtual Imzadi::Biped* SpawnMainCharacter() override;
	virtual void SpawnNPC(const std::string& type, const Imzadi::Vector3& position, const Imzadi::Quaternion& orientation) override;
};