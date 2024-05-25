#include "Hero.h"
#include "FollowCam.h"
#include "Game.h"
#include "RenderMesh.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"

using namespace Collision;

Hero::Hero()
{
}

/*virtual*/ Hero::~Hero()
{
}

/*virtual*/ bool Hero::Setup()
{
	std::string heroModelFile = "Models/Hero/Hero.render_mesh";
	this->renderMesh.SafeSet(Game::Get()->LoadAndPlaceRenderMesh(heroModelFile, this->restartLocation, Quaternion()));

	FollowCam* followCam = Game::Get()->SpawnEntity<FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCamera(Game::Get()->GetCamera());

	return true;
}

/*virtual*/ bool Hero::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ bool Hero::Tick(double deltaTime)
{
	// TODO: Respond to controller input to move the character around.

	return true;
}

/*virtual*/ bool Hero::GetTransform(Transform& transform)
{
	if (!this->renderMesh)
		return false;

	transform = this->renderMesh->GetObjectToWorldTransform();
	return true;
}