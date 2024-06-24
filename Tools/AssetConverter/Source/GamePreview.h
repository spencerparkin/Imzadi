#pragma once

#include <wx/window.h>
#include <wx/string.h>
#include "Game.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "RenderObjects/AnimatedMeshInstance.h"

class GamePreview : public Imzadi::Game
{
public:
	GamePreview(HINSTANCE instance);
	virtual ~GamePreview();

	virtual bool CreateRenderWindow() override;
	virtual void PumpWindowsMessages() override;
	virtual bool PostInit() override;
	virtual void Tick(Imzadi::TickPass tickPass) override;
	virtual bool PreShutdown() override;

	void SetAnimatingMesh(Imzadi::AnimatedMeshInstance* instance);

private:
	Imzadi::Reference<Imzadi::AnimatedMeshInstance> animatedMesh;
};