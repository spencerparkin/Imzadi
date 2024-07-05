#include "DeannaTroi.h"
#include "GameApp.h"

DeannaTroi::DeannaTroi()
{
}

/*virtual*/ DeannaTroi::~DeannaTroi()
{
}

/*virtual*/ bool DeannaTroi::Setup()
{
	std::string modelFile = "Models/DeannaTroi/Troi.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile, &this->restartLocation, &this->restartOrientation));

	if (!Hero::Setup())
		return false;

	return true;
}

/*virtual*/ bool DeannaTroi::Shutdown(bool gameShuttingDown)
{
	Hero::Shutdown(gameShuttingDown);

	return true;
}

/*virtual*/ bool DeannaTroi::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Hero::Tick(tickPass, deltaTime))
		return false;

	//...

	return true;
}