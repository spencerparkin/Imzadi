#include "Hero.h"
#include "FollowCam.h"
#include "Game.h"
#include "Assets/RenderMesh.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/Skeleton.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector2.h"
#include "Collision/Shapes/Capsule.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Collision/Command.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Collision/CollisionCache.h"
#include "Log.h"

using namespace Imzadi;

Hero::Hero()
{
	this->collisionShapeID = 0;
	this->groundShapeID = 0;
	this->cameraHandle = 0;
	this->maxMoveSpeed = 20.0;
	this->boundsQueryTaskID = 0;
	this->collisionQueryTaskID = 0;
	this->groundQueryTaskID = 0;
	this->inContactWithGround = false;
}

/*virtual*/ Hero::~Hero()
{
}

/*virtual*/ bool Hero::Setup()
{
	if (!this->renderMesh.Get())
	{
		IMZADI_LOG_ERROR("Derivatives of the Hero class need to initialize the render mesh.");
		return false;
	}

	Game::Get()->PushControllerUser("Hero");

	FollowCam* followCam = Game::Get()->SpawnEntity<FollowCam>();
	followCam->SetSubject(this);
	followCam->SetCamera(Game::Get()->GetCamera());

	this->cameraHandle = followCam->GetHandle();

	// TODO: Really should get capsule size from somewhere on disk.
	auto capsule = CapsuleShape::Create();
	capsule->SetVertex(0, Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	this->collisionShapeID = Game::Get()->GetCollisionSystem()->AddShape(capsule, 0);
	if (this->collisionShapeID == 0)
		return false;

	this->inContactWithGround = true;
	return true;
}

/*virtual*/ bool Hero::Shutdown(bool gameShuttingDown)
{
	Game::Get()->PopControllerUser();

	return true;
}

/*virtual*/ void Hero::AccumulateForces(Imzadi::Vector3& netForce)
{
	PhysicsEntity::AccumulateForces(netForce);

	Controller* controller = Game::Get()->GetController("Hero");
	if (!controller)
		return;

	if (this->inContactWithGround && controller->ButtonPressed(XINPUT_GAMEPAD_Y))
	{
		Vector3 jumpForce(0.0, 1000.0, 0.0);
		netForce += jumpForce;
	}
}

/*virtual*/ void Hero::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	PhysicsEntity::IntegrateVelocity(acceleration, deltaTime);

	if (this->inContactWithGround)
	{
		Reference<ReferenceCounted> followCamRef;
		HandleManager::Get()->GetObjectFromHandle(this->cameraHandle, followCamRef);
		auto followCam = dynamic_cast<FollowCam*>(followCamRef.Get());
		if (!followCam)
			return;

		Vector2 leftStick(0.0, 0.0);
		Controller* controller = Game::Get()->GetController("Hero");
		if (controller)
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

	CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

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

			objectToWorld.matrix.InterpolateOrientations(objectToWorld.matrix, targetOrienation, 0.2);
			objectToWorld.translation += this->velocity * deltaTime;

			this->renderMesh->SetObjectToWorldTransform(objectToWorld);

			auto command = ObjectToWorldCommand::Create();
			command->SetShapeID(this->collisionShapeID);
			command->objectToWorld = objectToWorld;
			collisionSystem->IssueCommand(command);

			auto boundsQuery = ShapeInBoundsQuery::Create();
			boundsQuery->SetShapeID(this->collisionShapeID);
			collisionSystem->MakeQuery(boundsQuery, this->boundsQueryTaskID);

			auto collisionQuery = CollisionQuery::Create();
			collisionQuery->SetShapeID(this->collisionShapeID);
			collisionSystem->MakeQuery(collisionQuery, this->collisionQueryTaskID);

			if (this->groundShapeID != 0)
			{
				auto objectToWorldQuery = ObjectToWorldQuery::Create();
				objectToWorldQuery->SetShapeID(this->groundShapeID);
				collisionSystem->MakeQuery(objectToWorldQuery, this->groundQueryTaskID);
			}

			auto animatedMesh = dynamic_cast<AnimatedMeshInstance*>(this->renderMesh.Get());
			if (animatedMesh)
			{
				Animation* animation = animatedMesh->GetAnimation();
				if (!this->inContactWithGround)
					animatedMesh->SetAnimation("Jumping");
				else
				{
					double threshold = 1.0;
					if (this->velocity.Length() < threshold)
						animatedMesh->SetAnimation("Idle");
					else
						animatedMesh->SetAnimation("Run");
				}
			}

			break;
		}
		case TickPass::MID_TICK:
		{
			auto animatedMesh = dynamic_cast<AnimatedMeshInstance*>(this->renderMesh.Get());
			if (animatedMesh)
				animatedMesh->AdvanceAnimation(deltaTime);

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

			if (this->collisionQueryTaskID)
			{
				Result* result = collisionSystem->ObtainQueryResult(this->collisionQueryTaskID);
				if (result)
				{
					auto collisionResult = dynamic_cast<CollisionQueryResult*>(result);
					if (collisionResult)
					{
						const ShapePairCollisionStatus* status = collisionResult->GetMostEgregiousCollision();
						if (!status)
						{
							this->inContactWithGround = false;
							this->groundShapeID = 0;
						}
						else
						{
							Vector3 separationDelta = status->GetSeparationDelta(this->collisionShapeID);
							Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
							objectToWorld.translation += separationDelta;
							this->renderMesh->SetObjectToWorldTransform(objectToWorld);
							this->velocity = this->velocity.RejectedFrom(separationDelta.Normalized());

							// TODO: How do we know it's the ground we're colliding with?  What if it's something else?
							//       For now, we're going to assume it's the ground.  I'll find a way when that assumption starts to fail.
							//       We probably need to narrow the query to only certain kinds of collisions shapes, such as those marked as the ground somehow.
							//       This would be a useful feature of the collision system, because we may want to support attack collisions, for example.
							this->inContactWithGround = true;
							this->groundShapeID = status->GetOtherShape(this->collisionShapeID);
						}
					}

					collisionSystem->Free<Result>(result);
				}
			}

			if (this->groundQueryTaskID)
			{
				Result* result = collisionSystem->ObtainQueryResult(this->groundQueryTaskID);
				if (result)
				{
					auto objectToWorldResult = dynamic_cast<ObjectToWorldResult*>(result);
					if (objectToWorldResult)
					{
						Vector3 groundMovementVector = objectToWorldResult->prevWorldToCurrentWorld.translation;
						Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
						objectToWorld.translation += groundMovementVector;
						this->renderMesh->SetObjectToWorldTransform(objectToWorld);
					}

					collisionSystem->Free<Result>(result);
				}
			}

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