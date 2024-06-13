#pragma once

#include <wx/app.h>

class Frame;
class GameEditor;

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