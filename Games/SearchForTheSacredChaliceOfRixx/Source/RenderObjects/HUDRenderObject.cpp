#include "HUDRenderObject.h"
#include "GameApp.h"
#include "Assets/GameProgress.h"

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

	// TODO: Also show inventory here.

	std::string hudText;
	hudText += std::format("Lives: {}", numLives);
	if (numLives == 0)
		hudText += " (GAME OVER!)";
	this->SetText(hudText);

	this->SetBackgroundAlpha(0.5);

	Imzadi::Transform scale;
	scale.SetIdentity();
	scale.matrix.SetNonUniformScale(Imzadi::Vector3(1.0, game->GetAspectRatio(), 1.0));

	Imzadi::Transform translate;
	translate.SetIdentity();
	translate.translation.SetComponents(-0.9, 0.9, -0.5);

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