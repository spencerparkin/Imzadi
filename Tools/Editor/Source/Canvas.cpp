#include "Canvas.h"
#include "App.h"
#include "GameEditor.h"

Canvas::Canvas(wxWindow* parent) : wxWindow(parent, wxID_ANY)
{
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
}

/*virtual*/ Canvas::~Canvas()
{
}

void Canvas::OnSize(wxSizeEvent& event)
{
	GameEditor* gameEditor = wxGetApp().GetGameEditor();
	if (gameEditor)
		gameEditor->NotifyWindowResized();
}