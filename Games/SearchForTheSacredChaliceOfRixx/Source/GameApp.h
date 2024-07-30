#pragma once

#include "Game.h"
#include "DialogSystem.h"
#include "Assets/GameProgress.h"

#define SHAPE_FLAG_TALKER			0x0000000100000000

class GameApp : public Imzadi::Game
{
public:
	GameApp(HINSTANCE instance);
	virtual ~GameApp();

	virtual bool PreInit() override;
	virtual bool PostInit() override;
	virtual bool PreShutdown() override;
	virtual bool PostShutdown() override;

	DialogSystem* GetDialogSystem() { return &this->dialogSystem; }
	GameProgress* GetGameProgress() { return this->gameProgress.Get(); }

private:
	void PerformLevelTransition(const std::string& nextLevel);

	bool GetGameAssetPath(std::filesystem::path& gameAssetPath);
	bool GetGameSavePath(std::filesystem::path& gameSavePath);

	DialogSystem dialogSystem;
	Imzadi::Reference<GameProgress> gameProgress;
};