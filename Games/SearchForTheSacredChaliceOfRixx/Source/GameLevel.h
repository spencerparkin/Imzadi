#pragma once

#include "Entities/Level.h"

class GameLevel : public Imzadi::Level
{
public:
	GameLevel();
	virtual ~GameLevel();

	virtual bool Setup() override;
	virtual Imzadi::Biped* SpawnMainCharacter() override;
};