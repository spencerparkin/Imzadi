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

	Imzadi::EventSystem* eventSystem = Imzadi::Game::Get()->GetEventSystem();
	eventSystem->RegisterEventListener("LevelTransition", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->PerformLevelTransition(event->GetName());
	}));

	eventSystem->SendEvent("LevelTransition", new Imzadi::Event("Level1"));

	if (!this->dialogSystem.Initialize())
	{
		IMZADI_LOG_ERROR("Failed to initialize the dialog system.");
		return false;
	}

	return true;
}

/*virtual*/ bool GameApp::PostShutdown()
{
	Game::PostShutdown();

	this->dialogSystem.Shutdown();

	Imzadi::LoggingSystem::Get()->ClearAllRoutes();

	return true;
}

void GameApp::PerformLevelTransition(const std::string& nextLevel)
{
	// Note that if we wanted to get fancy, we could transition
	// from level to level using a chain-reaction of events.
	// One event could initiate a character disolve.  Once the
	// character fades out, another could throw up a loading screen
	// and then start the level spawn.  Another event could trigger
	// once a frame processes without an entity spawn (signaling
	// the end of level load), then destroy the loading screen.
	// For now, I'm just going to do something quick and easy.

	this->ShutdownAllEntities();

	auto level = this->SpawnEntity<GameLevel>();
	level->SetLevelName(nextLevel);
}