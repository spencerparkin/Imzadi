#include "GameEditor.h"
#include "App.h"
#include "Frame.h"
#include "Canvas.h"

GameEditor::GameEditor(HINSTANCE instance) : Game(instance)
{
}

/*virtual*/ GameEditor::~GameEditor()
{
}

/*virtual*/ bool GameEditor::CreateRenderWindow()
{
	// Have the Imzadi engine use our window rather than create its own window.
	Canvas* canvas = wxGetApp().GetFrame()->GetCanvas();
	this->SetMainWindowHandle(canvas->GetHWND());
	return true;
}

/*virtual*/ void GameEditor::PumpWindowsMessages()
{
	// Do nothing here to prevent the engine from pumping windows messages.
	// wxWidgets will pump messages in its own way.
}