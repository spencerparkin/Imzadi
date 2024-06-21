#include "App.h"
#include "Frame.h"
#include "GamePreview.h"
#include "Log.h"
#include "LogWindow.h"

wxIMPLEMENT_APP(ConverterApp);

ConverterApp::ConverterApp()
{
	this->frame = nullptr;
}

/*virtual*/ ConverterApp::~ConverterApp()
{
}

/*virtual*/ bool ConverterApp::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(wxPoint(10, 10), wxSize(1200, 800));
	this->frame->Show();

	Imzadi::LoggingSystem::Get()->AddRoute(new LogWindowRoute());

	this->frame->SetFocus();

	IMZADI_LOG_INFO("Initializing Imzadi Game Engine...");

	HINSTANCE instanceHandle = ::GetModuleHandle(NULL);
	auto gamePreview = new GamePreview(instanceHandle);
	Imzadi::Game::Set(gamePreview);
	if (!gamePreview->Initialize())
	{
		IMZADI_LOG_ERROR("Initializatoin failed!");

		gamePreview->Shutdown();
		delete gamePreview;
		Imzadi::Game::Set(nullptr);
		return false;
	}
	
	IMZADI_LOG_INFO("Initialization succeeded!");

	// TODO: Shouldn't hard-code this path.
	gamePreview->GetAssetCache()->AddAssetFolder(R"(E:\ENG_DEV\Imzadi\Games\SearchForTheSacredChaliceOfRixx\Assets)");

	return true;
}

/*virtual*/ int ConverterApp::OnExit(void)
{
	IMZADI_LOG_INFO("Shutting down Imzadi Game Engine...");

	Imzadi::Game* game = Imzadi::Game::Get();
	if (game)
	{
		if (!game->Shutdown())
			IMZADI_LOG_ERROR("Shutdown failed!");
		else
			IMZADI_LOG_INFO("Shutdown succeeded!");

		delete game;
		Imzadi::Game::Set(nullptr);
	}

	Imzadi::LoggingSystem::Get()->ClearAllRoutes();

	return 0;
}