#pragma once

#include <wx/window.h>
#include "Game.h"

class GameEditor : public Imzadi::Game
{
public:
	GameEditor(HINSTANCE instance);
	virtual ~GameEditor();

	virtual bool CreateRenderWindow() override;
	virtual void PumpWindowsMessages() override;
};