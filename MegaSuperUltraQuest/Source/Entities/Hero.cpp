#include "Hero.h"
#include "FollowCam.h"
#include "Game.h"
#include "Assets/RenderMesh.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector2.h"

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
	Controller* controller = Game::Get()->GetController();

	Vector2 leftStick;
	controller->GetAnalogJoyStick(Controller::Side::LEFT, leftStick.x, leftStick.y);

	// TODO: We can't write the character movement code here until we can load the collision
	//       shapes and even draw them too, because we're not really moving the render mesh.
	//       What we're doing is moving the collision shape for the character and the rener
	//       mesh is slaved to the collision shape.

	return true;
}

/*virtual*/ bool Hero::GetTransform(Transform& transform)
{
	if (!this->renderMesh)
		return false;

	transform = this->renderMesh->GetObjectToWorldTransform();
	return true;
}