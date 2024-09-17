#include "HUDRenderObject.h"
#include "GameApp.h"
#include "Assets/GameProgress.h"
#include <algorithm>

HUDRenderObject::HUDRenderObject()
{
}

/*virtual*/ HUDRenderObject::~HUDRenderObject()
{
}

/*virtual*/ void HUDRenderObject::Prepare()
{
	auto game = (GameApp*)Imzadi::Game::Get();
	GameProgress* gameProgress = game->GetGameProgress();
	int numLives = gameProgress->GetNumLives();

	std::string hudText;
	hudText += std::format("Lives: {}", numLives);
	if (numLives == 0)
		hudText += " (GAME OVER!)";

	const std::unordered_map<std::string, uint32_t>& inventoryMap = gameProgress->GetInventoryMap();
	std::vector<std::string> inventoryStringArray;
	for (auto pair : inventoryMap)
		inventoryStringArray.push_back(std::format("\n{}s: {}", pair.first.c_str(), pair.second));
	std::sort(inventoryStringArray.begin(), inventoryStringArray.end());
	for (const std::string& inventoryString : inventoryStringArray)
		hudText += "\n" + inventoryString;

	this->SetText(hudText);

	this->SetBackgroundAlpha(0.5);

	Imzadi::Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Imzadi::Vector3(1.0, game->GetAspectRatio(), 1.0));

	Imzadi::Transform translate;
	translate.SetIdentity();
	translate.translation.SetComponents(-0.9, -0.9, -0.5);

	this->SetTransform(translate * scale);

	uint32_t flags = 0;
	flags |= Flag::ALWAYS_ON_TOP;
	flags |= Flag::LEFT_JUSTIFY;
	flags |= Flag::STICK_WITH_CAMERA_PROJ;
	flags |= Flag::MULTI_LINE;
	flags |= Flag::USE_NEWLINE_CHARS;
	flags |= Flag::DRAW_BACKGROUND;
	this->SetFlags(flags);

	TextRenderObject::Prepare();
}