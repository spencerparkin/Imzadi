#pragma once

#include "Game.h"

class GameApp : public Imzadi::Game
{
public:
	GameApp(HINSTANCE instance);
	virtual ~GameApp();

	virtual bool PreInit() override;
	virtual bool PostInit() override;
	virtual bool PostShutdown() override;
};