#include "Hero.h"
#include "FollowCam.h"
#include "Game.h"
#include "Assets/RenderMesh.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector2.h"
#include "Shapes/Capsule.h"
#include "Command.h"
#include "Query.h"
#include "Result.h"

using namespace Collision;

Hero::Hero()
{
	this->shapeID = 0;
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
	this->boundsQueryTaskID = 0;
	this->inContactWithGround = false;
}

/*virtual*/ Hero::~Hero()
{
}

/*virtual*/ bool Hero::Setup()
{
	Game::Get()->PushControllerUser("Hero");

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

	this->inContactWithGround = true;
	return true;
}

/*virtual*/ bool Hero::Shutdown(bool gameShuttingDown)
{
	Game::Get()->PopControllerUser();

	return true;
}

/*virtual*/ void Hero::AccumulateForces(Collision::Vector3& netForce)
{
	PhysicsEntity::AccumulateForces(netForce);

	// TODO: Maybe if the jump button is pushed we apply a jump force here?
}

/*virtual*/ void Hero::IntegrateVelocity(const Collision::Vector3& acceleration, double deltaTime)
{
	PhysicsEntity::IntegrateVelocity(acceleration, deltaTime);

	if (this->inContactWithGround)
	{
		auto followCam = dynamic_cast<FollowCam*>(ReferenceCounted::GetObjectFromHandle(this->cameraHandle));
		if (!followCam)
			return;

		Controller* controller = Game::Get()->GetController("Hero");
		if (!controller)
			return;

		Vector2 leftStick(0.0, 0.0);
		controller->GetAnalogJoyStick(Controller::Side::LEFT, leftStick.x, leftStick.y);

		Camera* camera = followCam->GetCamera();
		if (!camera)
			return;

		Transform cameraToWorld = camera->GetCameraToWorldTransform();

		Vector3 xAxis, yAxis, zAxis;
		cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

		Vector3 upVector(0.0, 1.0, 0.0);
		xAxis = xAxis.RejectedFrom(upVector).Normalized();
		zAxis = zAxis.RejectedFrom(upVector).Normalized();

		Vector3 moveDelta = (xAxis * leftStick.x - zAxis * leftStick.y) * this->maxMoveSpeed;

		// We don't stomp the Y-component here, because we still want
		// the effects of gravity applied on our character continuously.
		this->velocity.x = moveDelta.x;
		this->velocity.z = moveDelta.z;
	}
}

/*virtual*/ bool Hero::Tick(TickPass tickPass, double deltaTime)
{
	if (!PhysicsEntity::Tick(tickPass, deltaTime))
		return false;

	System* collisionSystem = Game::Get()->GetCollisionSystem();

	switch (tickPass)
	{
		case TickPass::PRE_TICK:
		{
			Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
			Matrix3x3 targetOrienation = objectToWorld.matrix;

			if (this->velocity.Length() > 0)
			{
				Vector3 xAxis, yAxis, zAxis;

				yAxis.SetComponents(0.0, 1.0, 0.0);
				zAxis = -this->velocity.RejectedFrom(yAxis).Normalized();
				xAxis = yAxis.Cross(zAxis);

				targetOrienation.SetColumnVectors(xAxis, yAxis, zAxis);
			}

			objectToWorld.matrix.InterpolateOrientations(objectToWorld.matrix, targetOrienation, 0.8);
			objectToWorld.translation += this->velocity * deltaTime;

			this->renderMesh->SetObjectToWorldTransform(objectToWorld);

			auto command = ObjectToWorldCommand::Create();
			command->SetShapeID(this->shapeID);
			command->objectToWorld = objectToWorld;
			collisionSystem->IssueCommand(command);

			auto boundsQuery = ShapeInBoundsQuery::Create();
			boundsQuery->SetShapeID(this->shapeID);
			collisionSystem->MakeQuery(boundsQuery, this->boundsQueryTaskID);

			break;
		}
		case TickPass::POST_TICK:
		{
			if (this->boundsQueryTaskID)
			{
				Result* result = collisionSystem->ObtainQueryResult(this->boundsQueryTaskID);
				if (result)
				{
					auto boolResult = dynamic_cast<BoolResult*>(result);
					if (boolResult && !boolResult->GetAnswer())
					{
						// Our character has gone out of bounds of the collision world!
						// This is probably because we fell off a platform into the infinite
						// void down below.  We have died!
						this->Reset();
					}

					collisionSystem->Free<Result>(result);
				}
			}

			// TODO: Solve constraints here.
			break;
		}
	}

	return true;
}

/*virtual*/ bool Hero::GetTransform(Transform& transform)
{
	if (!this->renderMesh)
		return false;

	transform = this->renderMesh->GetObjectToWorldTransform();
	return true;
}

/*virtual*/ void Hero::Reset()
{
	PhysicsEntity::Reset();

	Transform objectToWorld;
	objectToWorld.translation = this->restartLocation;
	objectToWorld.matrix.SetFromQuat(this->restartOrientation);
	this->renderMesh->SetObjectToWorldTransform(objectToWorld);
}