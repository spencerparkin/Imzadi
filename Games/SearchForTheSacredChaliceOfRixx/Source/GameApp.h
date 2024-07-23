#pragma once

#include "Game.h"
#include "DialogSystem.h"

#define SHAPE_FLAG_TALKER			0x0000000100000000

class GameApp : public Imzadi::Game
{
public:
	GameApp(HINSTANCE instance);
	virtual ~GameApp();

	virtual bool PreInit() override;
	virtual bool PostInit() override;
	virtual bool PostShutdown() override;

	DialogSystem* GetDialogSystem() { return &this->dialogSystem; }

private:
	void PerformLevelTransition(const std::string& nextLevel);

	DialogSystem dialogSystem;
};