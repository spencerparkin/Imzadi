#include "GameLevel.h"
#include "Characters/DeannaTroi.h"

GameLevel::GameLevel()
{
}

/*virtual*/ GameLevel::~GameLevel()
{
}

/*virtual*/ Imzadi::Hero* GameLevel::SpawnHero()
{
	return Imzadi::Game::Get()->SpawnEntity<DeannaTroi>();
}