#include "App.h"
#include "Frame.h"
#include "GamePreview.h"
#include "Log.h"
#include "LogWindow.h"
#include <wx/filename.h>

wxIMPLEMENT_APP(ConverterApp);

ConverterApp::ConverterApp()
{
	this->frame = nullptr;
	this->config = nullptr;
}

/*virtual*/ ConverterApp::~ConverterApp()
{
	delete this->config;
}

/*virtual*/ bool ConverterApp::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->config = new wxConfig("ImzadiConverterAppConfig");

	wxInitAllImageHandlers();

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

	gamePreview->GetAssetCache()->AddAssetFolder((const char*)this->GetAssetsRootFolder().c_str());

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

wxString ConverterApp::GetAssetsRootFolder()
{
	// TODO: Shouldn't hard-code this path.
	return R"(E:\ENG_DEV\Imzadi\Games\BenzoBonanza\Assets)";
}

wxString ConverterApp::MakeAssetFileReference(const wxString& assetFile)
{
	wxString relativeAssetPath = "?";

	const std::vector<std::filesystem::path>& assetFolderArray = Imzadi::Game::Get()->GetAssetCache()->GetAssetFolderArray();
	for (auto assetFolder : assetFolderArray)
	{
		wxFileName fileName(assetFile);
		if (!fileName.MakeRelativeTo(wxString(assetFolder.c_str())))
			continue;

		relativeAssetPath = fileName.GetFullPath();
		if (relativeAssetPath.Length() > 0 && ((const char*)relativeAssetPath.c_str())[0] != '.')
			break;
	}
	
	return relativeAssetPath;
}