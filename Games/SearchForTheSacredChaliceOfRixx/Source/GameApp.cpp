#include "GameApp.h"
#include "GameLevel.h"
#include "CustomAssetCache.h"

GameApp::GameApp(HINSTANCE instance) : Game(instance)
{
}

/*virtual*/ GameApp::~GameApp()
{
}

/*virtual*/ bool GameApp::PostInit()
{
	this->assetCache.Set(new CustomAssetCache());

	if (!Game::PostInit())
		return false;

	this->assetCache->AddAssetFolder(R"(E:\ENG_DEV\Imzadi\Games\SearchForTheSacredChaliceOfRixx\Assets)");		// TODO: Need to not hard-code a path here.

	auto level = this->SpawnEntity<GameLevel>();
	level->SetLevelNumber(1);		// TODO: Maybe remember what level we were on at the end of the last invocation of the game?

	return true;
}