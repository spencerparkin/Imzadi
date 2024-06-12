#pragma once

#include <wx/window.h>
#include <wx/string.h>
#include "Game.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"

class GameEditor : public Imzadi::Game
{
public:
	GameEditor(HINSTANCE instance);
	virtual ~GameEditor();

	virtual bool CreateRenderWindow() override;
	virtual void PumpWindowsMessages() override;
	virtual bool PostInit() override;

	bool Import(const aiScene* scene, wxString& error);

private:
};