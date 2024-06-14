#include "Canvas.h"
#include "App.h"
#include "Game.h"

Canvas::Canvas(wxWindow* parent) : wxWindow(parent, wxID_ANY)
{
	this->Bind(wxEVT_SIZE, &Canvas::OnSize, this);
}

/*virtual*/ Canvas::~Canvas()
{
}

void Canvas::OnSize(wxSizeEvent& event)
{
	Imzadi::Game* game = Imzadi::Game::Get();
	if (game)
		game->NotifyWindowResized();
}