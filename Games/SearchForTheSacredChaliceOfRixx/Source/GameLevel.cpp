#include "GameLevel.h"
#include "GameApp.h"
#include "Characters/DeannaTroi.h"
#include "Characters/LwaxanaTroi.h"
#include "Assets/GameLevelData.h"
#include "Entities/ZipLineEntity.h"
#include "Audio/System.h"
#include "Math/Interval.h"

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

	Imzadi::AudioSystem* audioSystem = Imzadi::Game::Get()->GetAudioSystem();
	audioSystem->AddAmbientSound({ "BlowingWind", "HowlingWind" }, Imzadi::Interval(25, 29), true);
	audioSystem->AddAmbientSound({ "OwlSound", "WindChimes" }, Imzadi::Interval(50, 100), false);

	return true;
}

/*virtual*/ bool GameLevel::SetupWithLevelData(Imzadi::LevelData* levelData)
{
	if (!Level::SetupWithLevelData(levelData))
		return false;

	auto gameLevelData = dynamic_cast<GameLevelData*>(levelData);
	if (!gameLevelData)
		return false;

	for (int i = 0; i < (signed)gameLevelData->GetZipLineArray().size(); i++)
	{
		ZipLine* zipLine = const_cast<ZipLine*>(gameLevelData->GetZipLineArray()[i].Get());
		ZipLineEntity* zipLineEntity = Imzadi::Game::Get()->SpawnEntity<ZipLineEntity>();
		zipLineEntity->SetZipLine(zipLine);
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