#include "App.h"
#include "Frame.h"
#include "GamePreview.h"
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
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(wxDefaultPosition, wxSize(800, 600));
	this->frame->Show();

	HINSTANCE instanceHandle = ::GetModuleHandle(NULL);
	auto gamePreview = new GamePreview(instanceHandle);
	Imzadi::Game::Set(gamePreview);
	if (!gamePreview->Initialize())
	{
		gamePreview->Shutdown();
		delete gamePreview;
		Imzadi::Game::Set(nullptr);
		return false;
	}
	
	// TODO: Make this something the user can configure.
	gamePreview->GetAssetCache()->AddAssetFolder(R"(E:\ENG_DEV\Imzadi\Games\SearchForTheSacredChaliceOfRixx\Assets)");

	Imzadi::AnimatedMeshInstance::SetRenderSkeletons(true);

	return true;
}

/*virtual*/ int ConverterApp::OnExit(void)
{
	Imzadi::Game* game = Imzadi::Game::Get();
	if (game)
	{
		game->Shutdown();
		delete game;
		Imzadi::Game::Set(nullptr);
	}

	return 0;
}