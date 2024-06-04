#include "Hero.h"
#include "FollowCam.h"
#include "Game.h"
#include "Assets/RenderMesh.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector2.h"
#include "Shapes/Capsule.h"
#include "Command.h"

using namespace Collision;

Hero::Hero()
{
	this->shapeID = 0;
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
}

/*virtual*/ Hero::~Hero()
{
}

/*virtual*/ bool Hero::Setup()
{
	std::string heroModelFile = "Models/Hero/Hero.render_mesh";
	this->renderMesh.SafeSet(Game::Get()->LoadAndPlaceRenderMesh(heroModelFile, this->restartLocation, this->restartOrientation));

	FollowCam* followCam = Game::Get()->SpawnEntity<FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCamera(Game::Get()->GetCamera());

	this->cameraHandle = followCam->GetHandle();

	auto capsule = CapsuleShape::Create();
	capsule->SetVertex(0, Vector3(0.0, 2.0, 0.0));
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

	Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
	Matrix3x3 targetOrienation = objectToWorld.matrix;

	auto followCam = dynamic_cast<FollowCam*>(ReferenceCounted::GetObjectFromHandle(this->cameraHandle));
	if (followCam)
	{
		Camera* camera = followCam->GetCamera();
		Transform cameraToWorld = camera->GetCameraToWorldTransform();

		Vector3 xAxis, yAxis, zAxis;
		cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

		Vector3 upVector(0.0, 1.0, 0.0);
		xAxis = xAxis.RejectedFrom(upVector);
		zAxis = zAxis.RejectedFrom(upVector);

		Vector3 moveDelta = (xAxis * leftStick.x - zAxis * leftStick.y) * this->maxMoveSpeed * deltaTime;

		objectToWorld.translation += moveDelta;

		if (moveDelta.Length() > 0)
		{
			zAxis = -moveDelta.Normalized();
			yAxis = upVector.RejectedFrom(zAxis);
			xAxis = yAxis.Cross(zAxis);

			targetOrienation.SetColumnVectors(xAxis, yAxis, zAxis);
		}
	}

	objectToWorld.matrix.InterpolateOrientations(objectToWorld.matrix, targetOrienation, 0.8);

	this->renderMesh->SetObjectToWorldTransform(objectToWorld);

	auto command = ObjectToWorldCommand::Create();
	command->SetShapeID(this->shapeID);
	command->objectToWorld = objectToWorld;
	Game::Get()->GetCollisionSystem()->IssueCommand(command);

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