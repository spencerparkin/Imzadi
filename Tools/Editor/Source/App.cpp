#include "App.h"
#include "Frame.h"
#include "GameEditor.h"

wxIMPLEMENT_APP(EditorApp);

EditorApp::EditorApp()
{
	this->frame = nullptr;
	this->gameEditor = nullptr;
}

/*virtual*/ EditorApp::~EditorApp()
{
}

/*virtual*/ bool EditorApp::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(wxDefaultPosition, wxSize(800, 600));
	this->frame->Show();

	HINSTANCE instanceHandle = ::GetModuleHandle(NULL);
	this->gameEditor = new GameEditor(instanceHandle);
	Imzadi::Game::Set(this->gameEditor);
	if (!this->gameEditor->Initialize())
	{
		this->gameEditor->Shutdown();
		delete this->gameEditor;
		this->gameEditor = nullptr;
		return false;
	}
	
	return true;
}

/*virtual*/ int EditorApp::OnExit(void)
{
	if (this->gameEditor)
	{
		this->gameEditor->Shutdown();
		delete this->gameEditor;
		Imzadi::Game::Set(nullptr);
	}

	return 0;
}