#include "Biped.h"
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
#include "Collision/CollisionCache.h"
#include "Log.h"

using namespace Imzadi;

Biped::Biped()
{
	this->collisionShapeID = 0;
	this->groundShapeID = 0;
	this->boundsQueryTaskID = 0;
	this->worldSurfaceCollisionQueryTaskID = 0;
	this->groundQueryTaskID = 0;
	this->inContactWithGround = false;
	this->canRestart = true;
}

/*virtual*/ Biped::~Biped()
{
}

/*virtual*/ bool Biped::OwnsCollisionShape(ShapeID shapeID) const
{
	return shapeID == this->collisionShapeID;
}

/*virtual*/ uint64_t Biped::GetAdditionalUserFlagsForCollisionShape()
{
	return 0;
}

/*virtual*/ bool Biped::Setup()
{
	if (!this->renderMesh.Get())
	{
		IMZADI_LOG_ERROR("Derivatives of the Biped class need to initialize the render mesh.");
		return false;
	}

	// TODO: Really should get capsule size from somewhere on disk.
	auto capsule = CapsuleShape::Create();
	capsule->SetVertex(0, Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | this->GetAdditionalUserFlagsForCollisionShape());
	this->collisionShapeID = Game::Get()->GetCollisionSystem()->AddShape(capsule, 0);
	if (this->collisionShapeID == 0)
		return false;

	this->inContactWithGround = false;
	return true;
}

/*virtual*/ bool Biped::Shutdown()
{
	PhysicsEntity::Shutdown();

	return true;
}

/*virtual*/ void Biped::AdjustFacingDirection(double deltaTime)
{
	// Make sure the render mesh faces the direction we're moving.
	Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
	Matrix3x3 targetOrientation = objectToWorld.matrix;

	if (this->velocity.Length() > 0)
	{
		Vector3 xAxis, yAxis, zAxis;

		yAxis.SetComponents(0.0, 1.0, 0.0);
		zAxis = -this->velocity.RejectedFrom(yAxis).Normalized();
		xAxis = yAxis.Cross(zAxis);

		targetOrientation.SetColumnVectors(xAxis, yAxis, zAxis);
	}

	objectToWorld.matrix.InterpolateOrientations(objectToWorld.matrix, targetOrientation, 0.2);

	this->renderMesh->SetObjectToWorldTransform(objectToWorld);
}

/*virtual*/ void Biped::IntegratePosition(double deltaTime)
{
	Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
	objectToWorld.translation += this->velocity * deltaTime;
	this->renderMesh->SetObjectToWorldTransform(objectToWorld);
}

/*virtual*/ bool Biped::Tick(TickPass tickPass, double deltaTime)
{
	if (!PhysicsEntity::Tick(tickPass, deltaTime))
		return false;

	CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

	switch (tickPass)
	{
		case TickPass::COMMAND_TICK:
		{
			this->AdjustFacingDirection(deltaTime);
			this->IntegratePosition(deltaTime);

			// Make sure that the collision shape transform for the biped matches the biped's render mesh transform.
			auto command = ObjectToWorldCommand::Create();
			command->SetShapeID(this->collisionShapeID);
			command->objectToWorld = this->renderMesh->GetObjectToWorldTransform();
			collisionSystem->IssueCommand(command);

			break;
		}
		case TickPass::QUERY_TICK:
		{
			// Kick-off the queries we'll need later to resolve collision constraints.

			auto boundsQuery = ShapeInBoundsQuery::Create();
			boundsQuery->SetShapeID(this->collisionShapeID);
			collisionSystem->MakeQuery(boundsQuery, this->boundsQueryTaskID);

			auto worldSurfaceCollisionQuery = CollisionQuery::Create();
			worldSurfaceCollisionQuery->SetShapeID(this->collisionShapeID);
			worldSurfaceCollisionQuery->SetUserFlagsMask(IMZADI_SHAPE_FLAG_WORLD_SURFACE);
			collisionSystem->MakeQuery(worldSurfaceCollisionQuery, this->worldSurfaceCollisionQueryTaskID);

			if (this->groundShapeID != 0)
			{
				auto objectToWorldQuery = ObjectToWorldQuery::Create();
				objectToWorldQuery->SetShapeID(this->groundShapeID);
				collisionSystem->MakeQuery(objectToWorldQuery, this->groundQueryTaskID);
			}

			break;
		}
		case TickPass::PARALLEL_TICK:
		{
			// Make sure we're playing an appropriate animation and pump the animation system.

			auto animatedMesh = dynamic_cast<AnimatedMeshInstance*>(this->renderMesh.Get());
			if (animatedMesh)
			{
				Animation* animation = animatedMesh->GetAnimation();
				if (!this->inContactWithGround)
					animatedMesh->SetAnimation(this->GetAnimName(AnimType::JUMP));
				else
				{
					double threshold = 1.0;
					if (this->velocity.Length() < threshold)
						animatedMesh->SetAnimation(this->GetAnimName(AnimType::IDLE));
					else
						animatedMesh->SetAnimation(this->GetAnimName(AnimType::RUN));
				}

				// TODO: If running, make sure that animation speed matches the speed we're moving
				//       so that we don't skate across the ground.

				animatedMesh->AdvanceAnimation(deltaTime);
			}

			break;
		}
		case TickPass::RESULT_TICK:
		{
			if (this->boundsQueryTaskID)
			{
				bool characterDied = false;
				Result* result = collisionSystem->ObtainQueryResult(this->boundsQueryTaskID);
				if (result)
				{
					auto boolResult = dynamic_cast<BoolResult*>(result);
					if (boolResult && !boolResult->GetAnswer())
						characterDied = true;

					collisionSystem->Free(result);
				}

				if (characterDied)
				{
					if (this->canRestart)
						this->Reset();
					else
						return false;
				}
			}

			if (this->worldSurfaceCollisionQueryTaskID)
			{
				Result* result = collisionSystem->ObtainQueryResult(this->worldSurfaceCollisionQueryTaskID);
				if (result)
				{
					auto collisionResult = dynamic_cast<CollisionQueryResult*>(result);
					if (collisionResult)
						this->HandleWorldSurfaceCollisionResult(collisionResult);

					collisionSystem->Free(result);
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
						// TODO: I think we'll need this to update a reference frame.
						//       Can we have our character always move relative to the
						//       ground's reference frame, and can we know how to switch
						//       between reference frames?
						//objectToWorldResult->objectToWorld
					}

					collisionSystem->Free(result);
				}
			}

			break;
		}
	}

	return true;
}

/*virtual*/ std::string Biped::GetAnimName(AnimType animType)
{
	switch (animType)
	{
	case AnimType::RUN:
		return "Run";
	case AnimType::IDLE:
		return "Idle";
	case AnimType::JUMP:
		return "Jumping";
	}

	return "?";
}

void Biped::HandleWorldSurfaceCollisionResult(CollisionQueryResult* collisionResult)
{
	this->inContactWithGround = false;
	this->groundShapeID = 0;

	if (collisionResult->GetCollisionStatusArray().size() == 0)
		return;

	Vector3 averageSeperationDelta(0.0, 0.0, 0.0);
	Vector3 approximateGroundNormal;
	for (const auto& collisionStatus : collisionResult->GetCollisionStatusArray())
	{
		ShapeID otherShapeID = collisionStatus->GetOtherShape(this->collisionShapeID);
		
		Vector3 separationDelta = collisionStatus->GetSeparationDelta(this->collisionShapeID);
		averageSeperationDelta += separationDelta;

		// We really need a contact normal here, but do this for now.
		Vector3 upVector(0.0, 1.0, 0.0);
		double angle = upVector.AngleBetween(separationDelta.Normalized());
		if (angle < M_PI / 4.0)
		{
			this->inContactWithGround = true;
			this->groundShapeID = otherShapeID;
			approximateGroundNormal = separationDelta.Normalized();
		}
	}

	averageSeperationDelta /= float(collisionResult->GetCollisionStatusArray().size());

	Transform objectToWorld = this->renderMesh->GetObjectToWorldTransform();
	objectToWorld.translation += averageSeperationDelta;
	this->renderMesh->SetObjectToWorldTransform(objectToWorld);

	if (this->groundShapeID != 0)
		this->velocity = this->velocity.RejectedFrom(approximateGroundNormal);
}

/*virtual*/ bool Biped::GetTransform(Transform& transform) const
{
	if (!this->renderMesh)
		return false;

	transform = this->renderMesh->GetObjectToWorldTransform();
	return true;
}

/*virtual*/ bool Biped::SetTransform(const Transform& transform)
{
	if (!this->renderMesh)
		return false;

	this->renderMesh->SetObjectToWorldTransform(transform);
	return true;
}

/*virtual*/ void Biped::Reset()
{
	PhysicsEntity::Reset();

	Transform objectToWorld;
	objectToWorld.translation = this->restartLocation;
	objectToWorld.matrix.SetFromQuat(this->restartOrientation);
	this->renderMesh->SetObjectToWorldTransform(objectToWorld);
}