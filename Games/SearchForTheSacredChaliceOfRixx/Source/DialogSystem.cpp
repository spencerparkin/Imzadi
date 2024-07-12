#include "DialogSystem.h"
#include "Assets/DialogData.h"
#include "CustomAssetCache.h"
#include "GameApp.h"
#include "Log.h"

DialogSystem::DialogSystem()
{
}

/*virtual*/ DialogSystem::~DialogSystem()
{
}

bool DialogSystem::Initialize()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	auto assetCache = game->GetAssetCache();

	Imzadi::Reference<Imzadi::Asset> asset;
	if (!assetCache->LoadAsset("Dialog/Dialog.dialog", asset))
	{
		IMZADI_LOG_ERROR("Failed to load dialog data asset.");
		return false;
	}

	this->dialogData.SafeSet(asset.Get());
	if (!this->dialogData)
	{
		IMZADI_LOG_ERROR("Whatever loaded for the dialog data wasn't dialog data.");
		return false;
	}

	return true;
}

bool DialogSystem::Shutdown()
{
	this->dialogData.Reset();
	return true;
}