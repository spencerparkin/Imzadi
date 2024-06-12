#pragma once

#include <wx/app.h>

class Frame;
class GameEditor;

// Admittedly, there isn't much editing going on here in the editor...yet, I suppose.
// The main goal here is to have a tool that can convert assets from an art program
// into assets that can be consumed by the Imzadi Game Engine.
class EditorApp : public wxApp
{
public:
	EditorApp();
	virtual ~EditorApp();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Frame* GetFrame() { return this->frame; }
	GameEditor* GetGameEditor() { return this->gameEditor; }

private:
	Frame* frame;
	GameEditor* gameEditor;
};

wxDECLARE_APP(EditorApp);