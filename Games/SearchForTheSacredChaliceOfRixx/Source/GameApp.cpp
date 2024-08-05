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

	std::filesystem::path gameAssetPath;
	if (!this->GetGameAssetPath(gameAssetPath))
		return false;

	this->assetCache->AddAssetFolder(gameAssetPath.string());

	std::filesystem::path gameSavePath;
	if (!this->GetGameSavePath(gameSavePath))
		return false;

	Imzadi::Reference<Imzadi::Asset> asset;
	this->assetCache->LoadAsset(gameSavePath.string(), asset);
	if (asset.Get())
		this->gameProgress.SafeSet(asset.Get());
	if (!this->gameProgress)
		this->gameProgress.Set(new GameProgress());

	Imzadi::EventSystem* eventSystem = Imzadi::Game::Get()->GetEventSystem();
	eventSystem->RegisterEventListener("LevelTransition", new Imzadi::LambdaEventListener([=](const Imzadi::Event* event) {
		this->PerformLevelTransition(event->GetName());
	}));

	std::string levelName = this->gameProgress->GetLevelName();
	eventSystem->SendEvent("LevelTransition", new Imzadi::Event(levelName));

	if (!this->dialogSystem.Initialize())
	{
		IMZADI_LOG_ERROR("Failed to initialize the dialog system.");
		return false;
	}

	if (!Imzadi::Game::Get()->GetAudioSystem()->LoadAudioDirectory("Audio", true))
	{
		IMZADI_LOG_ERROR("Failed to load audio directory.");
		return false;
	}

	return true;
}

bool GameApp::GetGameAssetPath(std::filesystem::path& gameAssetPath)
{
	gameAssetPath = R"(E:\ENG_DEV\Imzadi\Games\SearchForTheSacredChaliceOfRixx\Assets)";	// TODO: Need to not hard-code a path here.
	return true;
}

bool GameApp::GetGameSavePath(std::filesystem::path& gameSavePath)
{
	std::filesystem::path gameAssetPath;
	if (!this->GetGameAssetPath(gameAssetPath))
		return false;

	std::filesystem::path gameSaveFolder = gameAssetPath / "Progress";
	if (!std::filesystem::exists(gameSaveFolder) && !std::filesystem::create_directory(gameSaveFolder))
		return false;

	std::string userName;
	char userNameBuffer[512];
	DWORD userNameBufferSize = sizeof(userNameBuffer);
	if (::GetUserNameA(userNameBuffer, &userNameBufferSize))
		userName = userNameBuffer;
	else
		userName = "Anonymous";

	while (true)
	{
		size_t i = userName.find('.');
		if (i == std::string::npos)
			break;

		userName = userName.replace(i, 1, "_");
	}

	gameSavePath = gameSaveFolder / std::format("{}.progress", userName.c_str());
	return true;
}

/*virtual*/ bool GameApp::PreShutdown()
{
	Game::PreShutdown();

	if (this->gameProgress)
	{
		std::filesystem::path gameSavePath;
		if (this->GetGameSavePath(gameSavePath))
		{
			Imzadi::Reference<Imzadi::Asset> asset(this->gameProgress.Get());
			this->GetAssetCache()->SaveAsset(gameSavePath.string(), asset);
		}
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

	this->gameProgress->SetLevelName(nextLevel);
}