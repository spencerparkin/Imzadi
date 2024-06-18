#include "App.h"
#include "Frame.h"
#include "GamePreview.h"
#include "Log.h"
#include "RenderObjects/AnimatedMeshInstance.h"

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
	Log::Get()->AddRoute("default", new LogFileRoute());

	if (!wxApp::OnInit())
	{
		LOG("wxApp::OnInit() failed!");
		return false;
	}

	this->frame = new Frame(wxPoint(10, 10), wxSize(800, 600));
	this->frame->Show();
	
	Log::Get()->AddRoute("log_window", new LogWindowRoute());

	Imzadi::Error::Get()->RegisterErrorCapture("log", Log::Get());

	this->frame->SetFocus();

	LOG("Initializing Imzadi Game Engine...");

	HINSTANCE instanceHandle = ::GetModuleHandle(NULL);
	auto gamePreview = new GamePreview(instanceHandle);
	Imzadi::Game::Set(gamePreview);
	if (!gamePreview->Initialize())
	{
		LOG("Initializatoin failed!");

		gamePreview->Shutdown();
		delete gamePreview;
		Imzadi::Game::Set(nullptr);
		return false;
	}
	
	LOG("Initialization succeeded!");

	// TODO: Shouldn't hard-code this path.
	gamePreview->GetAssetCache()->AddAssetFolder(R"(E:\ENG_DEV\Imzadi\Games\SearchForTheSacredChaliceOfRixx\Assets)");

	Imzadi::AnimatedMeshInstance::SetRenderSkeletons(true);

	return true;
}

/*virtual*/ int ConverterApp::OnExit(void)
{
	LOG("Shutting down Imzadi Game Engine...");

	Imzadi::Game* game = Imzadi::Game::Get();
	if (game)
	{
		if (!game->Shutdown())
			LOG("Shutdown failed!");
		else
			LOG("Shutdown succeeded!");

		delete game;
		Imzadi::Game::Set(nullptr);
	}

	Log::Get()->RemoveAllRoutes();

	return 0;
}