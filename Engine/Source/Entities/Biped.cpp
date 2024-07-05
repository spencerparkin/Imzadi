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
#include "Collision/Result.h"
#include "Collision/CollisionCache.h"
#include "Log.h"

using namespace Imzadi;

Biped::Biped()
{
	this->collisionShapeID = 0;
	this->groundShapeID = 0;
	this->boundsQueryTaskID = 0;
	this->collisionQueryTaskID = 0;
	this->groundQueryTaskID = 0;
	this->inContactWithGround = false;
	this->canRestart = true;
}

/*virtual*/ Biped::~Biped()
{
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

/*virtual*/ bool Biped::Tick(TickPass tickPass, double deltaTime)
{
	if (!PhysicsEntity::Tick(tickPass, deltaTime))
		return false;

	CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

	switch (tickPass)
	{
		case TickPass::COMMAND_TICK:
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
			objectToWorld.translation += this->velocity * deltaTime;

			this->renderMesh->SetObjectToWorldTransform(objectToWorld);

			// Make sure that the collision shape transform for the biped matches the biped's render mesh transform.
			auto command = ObjectToWorldCommand::Create();
			command->SetShapeID(this->collisionShapeID);
			command->objectToWorld = objectToWorld;
			collisionSystem->IssueCommand(command);

			break;
		}
		case TickPass::QUERY_TICK:
		{
			// Kick-off the queries we'll need later to resolve collision constraints.

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
					animatedMesh->SetAnimation("Jumping");
				else
				{
					double threshold = 1.0;
					if (this->velocity.Length() < threshold)
						animatedMesh->SetAnimation("Idle");
					else
						animatedMesh->SetAnimation("Run");
				}

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

/*virtual*/ bool Biped::GetTransform(Transform& transform)
{
	if (!this->renderMesh)
		return false;

	transform = this->renderMesh->GetObjectToWorldTransform();
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