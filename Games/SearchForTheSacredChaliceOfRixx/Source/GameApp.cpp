#include "GameApp.h"
#include "GameLevel.h"
#include "CustomAssetCache.h"
#include "Log.h"

GameApp::GameApp(HINSTANCE instance) : Game(instance)
{
	lstrcpy(this->windowTitle, "Search for the Sacred Chalice of Rixx");
}

/*virtual*/ GameApp::~GameApp()
{
}

/*virtual*/ bool GameApp::PreInit()
{
	if (!Game::PreInit())
		return false;

#if defined _DEBUG
	Imzadi::LoggingSystem::Get()->AddRoute(new Imzadi::LogConsoleRoute());
#endif

	return true;
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

/*virtual*/ bool GameApp::PostShutdown()
{
	Game::PostShutdown();

	Imzadi::LoggingSystem::Get()->ClearAllRoutes();

	return true;
}