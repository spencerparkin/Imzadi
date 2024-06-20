#pragma once

#include "Entities/Level.h"

class GameLevel : public Imzadi::Level
{
public:
	GameLevel();
	virtual ~GameLevel();

	virtual Imzadi::Hero* SpawnHero() override;
};