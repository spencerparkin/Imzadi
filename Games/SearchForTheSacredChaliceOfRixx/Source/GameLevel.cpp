#include "GameLevel.h"
#include "GameApp.h"
#include "Characters/DeannaTroi.h"
#include "Characters/LwaxanaTroi.h"

GameLevel::GameLevel()
{
}

/*virtual*/ GameLevel::~GameLevel()
{
}

/*virtual*/ Imzadi::Biped* GameLevel::SpawnMainCharacter()
{
	return Imzadi::Game::Get()->SpawnEntity<DeannaTroi>();
}

/*virtual*/ bool GameLevel::Setup()
{
	if (!Level::Setup())
		return false;

	if (this->GetLevelName() == "Level1")
	{
		auto lwaxana = Imzadi::Game::Get()->SpawnEntity<LwaxanaTroi>();
		lwaxana->SetRestartLocation(Imzadi::Vector3(-6.814, 1.6, -105.338));
		lwaxana->SetRestartOrientation(Imzadi::Quaternion());
	}

	return true;
}

/*virtual*/ bool GameLevel::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Level::Tick(tickPass, deltaTime))
		return false;

	auto game = (GameApp*)Imzadi::Game::Get();
	game->GetDialogSystem()->Tick();
	return true;
}