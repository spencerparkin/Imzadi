#include "Hero.h"
#include "FollowCam.h"
#include "Game.h"
#include "Assets/RenderMesh.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector2.h"
#include "Shapes/Capsule.h"

using namespace Collision;

Hero::Hero()
{
	this->shapeID = 0;
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

	auto capsule = CapsuleShape::Create();
	capsule->SetVertex(0, Vector3(0.0, 0.0, 0.0));
	capsule->SetVertex(1, Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(2.0);
	this->shapeID = Game::Get()->GetCollisionSystem()->AddShape(capsule, 0);
	if (this->shapeID == 0)
		return false;

	return true;
}

/*virtual*/ bool Hero::Shutdown(bool gameShuttingDown)
{
	// No need to do anything here.  We'll get cleaned up when
	// the scene and collision system is cleaned up.
	return true;
}

/*virtual*/ bool Hero::Tick(double deltaTime)
{
	Controller* controller = Game::Get()->GetController();

	Vector2 leftStick;
	controller->GetAnalogJoyStick(Controller::Side::LEFT, leftStick.x, leftStick.y);

	// TODO: Psuedo-code...
	//       - Do we have a collision query result?
	//       - No: Make one and bail.
	//       - Yes:
	//         - Solve constraints and apply deltas to this render mesh's object-to-world transform.
	//         - Issue command setting collision shape object-to-world as a function of render mesh's object-to-world and a relative transform.
	//         - Make collision query for next tick.

	return true;
}

/*virtual*/ bool Hero::GetTransform(Transform& transform)
{
	if (!this->renderMesh)
		return false;

	transform = this->renderMesh->GetObjectToWorldTransform();
	return true;
}