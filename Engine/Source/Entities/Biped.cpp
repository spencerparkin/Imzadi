#include "Biped.h"
#include "Game.h"
#include "Assets/RenderMesh.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/Skeleton.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector2.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Collision/Command.h"
#include "Collision/Query.h"
#include "Collision/CollisionCache.h"
#include "Log.h"

using namespace Imzadi;

Biped::Biped()
{
	this->animationMode = AnimationMode::BASIC_PLATFORMING;
	this->collisionShapeID = 0;
	this->groundShapeID = 0;
	this->boundsQueryTaskID = 0;
	this->worldSurfaceCollisionQueryTaskID = 0;
	this->groundQueryTaskID = 0;
	this->groundSurfaceQueryTaskID = 0;
	this->inContactWithGround = false;
	this->canRestart = true;
	this->mass = 1.0;
	this->continuouslyUpdatePlatformTransform = true;
}

/*virtual*/ Biped::~Biped()
{
}

/*virtual*/ bool Biped::OwnsCollisionShape(Collision::ShapeID shapeID) const
{
	return shapeID == this->collisionShapeID;
}

/*virtual*/ Collision::ShapeID Biped::GetGroundContactShape() const
{
	if (!this->inContactWithGround)
		return 0;

	return this->groundShapeID;
}

/*virtual*/ bool Biped::Setup()
{
	if (!this->renderMesh.Get())
	{
		IMZADI_LOG_ERROR("Derivatives of the Biped class need to initialize the render mesh.");
		return false;
	}

	// TODO: Really should get capsule size from somewhere on disk.
	auto capsule = new Collision::CapsuleShape();
	this->ConfigureCollisionCapsule(capsule);
	this->collisionShapeID = Game::Get()->GetCollisionSystem()->AddShape(capsule, 0);
	if (this->collisionShapeID == 0)
		return false;

	this->velocity.SetComponents(0.0, 0.0, 0.0);
	this->mass = 1.0;
	this->platformToWorld.SetIdentity();
	this->objectToPlatform = this->restartTransformObjectToWorld;
	this->inContactWithGround = false;
	this->groundShapeID = 0;
	return true;
}

/*virtual*/ bool Biped::Shutdown()
{
	Game::Get()->GetScene()->RemoveRenderObject(this->renderMesh->GetName());
	Game::Get()->GetCollisionSystem()->RemoveShape(this->collisionShapeID);
	this->collisionShapeID = 0;

	Entity::Shutdown();
	return true;
}

/*virtual*/ void Biped::ConfigureCollisionCapsule(Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Vector3(0.0, 1.0, 0.0));
	capsule->SetVertex(1, Vector3(0.0, 5.0, 0.0));
	capsule->SetRadius(1.0);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY);
}

/*virtual*/ std::string Biped::GetInfo() const
{
	Transform transform;
	this->GetTransform(transform);

	std::string info = std::format("Position: <{}, {}, {}>", transform.translation.x, transform.translation.y, transform.translation.z);
	return info;
}

/*virtual*/ void Biped::AdjustFacingDirection(double deltaTime)
{
	// Make sure the render mesh faces the direction we're moving.
	Matrix3x3 targetOrientation = this->objectToPlatform.matrix;

	if (this->velocity.Length() > 0)
	{
		Vector3 xAxis, yAxis, zAxis;

		yAxis.SetComponents(0.0, 1.0, 0.0);
		zAxis = -this->velocity.RejectedFrom(yAxis).Normalized();
		xAxis = yAxis.Cross(zAxis);

		targetOrientation.SetColumnVectors(xAxis, yAxis, zAxis);
	}

	this->objectToPlatform.matrix.InterpolateOrientations(this->objectToPlatform.matrix, targetOrientation, 0.2);
}

/*virtual*/ void Biped::AccumulateForces(Vector3& netForce)
{
	// TODO: Since we're working in platform space, this may not be the proper down-vector.
	Vector3 downVector(0.0, -1.0, 0.0);
	Vector3 gravityForce = downVector * this->mass * Game::Get()->GetGravity();
	netForce += gravityForce;
}

/*virtual*/ void Biped::IntegrateVelocity(const Vector3& acceleration, double deltaTime)
{
	this->velocity += acceleration * deltaTime;
}

/*virtual*/ void Biped::IntegratePosition(double deltaTime)
{
	this->objectToPlatform.translation += this->velocity * deltaTime;
}

/*virtual*/ uint32_t Biped::TickOrder() const
{
	return 1;
}

/*virtual*/ bool Biped::Tick(TickPass tickPass, double deltaTime)
{
	if (!Entity::Tick(tickPass, deltaTime))
		return false;

	Collision::System* collisionSystem = Game::Get()->GetCollisionSystem();

	switch (tickPass)
	{
		case TickPass::MOVE_UNCONSTRAINTED:
		{
			Vector3 netForce(0.0, 0.0, 0.0);
			this->AccumulateForces(netForce);
			Vector3 acceleration = netForce / this->mass;
			this->IntegrateVelocity(acceleration, deltaTime);
			this->IntegratePosition(deltaTime);
			this->AdjustFacingDirection(deltaTime);

			// If we're touching the ground, we want our character to move with the ground.
			// The moment we jump off the ground, we should be moving on our own, by the way,
			// and not influenced by the moving ground at all.  Hmmm, when we jump off a moving
			// platform, though, shouldn't we inherit some momentum from the platform?
			if (this->groundShapeID != 0 && this->inContactWithGround)
			{
				// Stalling for a minor query like this isn't so bad.  It's the big queries
				// where I'm hoping to get some sort of speed-up by doing them asynchronously.
				auto objectToWorldQuery = new Collision::ObjectToWorldQuery();
				objectToWorldQuery->SetShapeID(this->groundShapeID);
				collisionSystem->MakeQuery(objectToWorldQuery, this->groundQueryTaskID);
				collisionSystem->FlushAllTasks();
				Collision::Result* result = collisionSystem->ObtainQueryResult(this->groundQueryTaskID);
				if (result)
				{
					auto objectToWorldResult = dynamic_cast<Collision::ObjectToWorldResult*>(result);
					if (objectToWorldResult)
					{
						if (this->continuouslyUpdatePlatformTransform)
							this->platformToWorld = objectToWorldResult->objectToWorld;
						else
						{
							// TODO: Here we might use the transform in a different way to, say,
							//       apply friction to the character.
						}
					}

					delete result;
				}
			}

			Transform objectToWorld = this->platformToWorld * this->objectToPlatform;
			this->SetTransform(objectToWorld);

			break;
		}
		case TickPass::SUBMIT_COLLISION_QUERIES:
		{
			// Kick-off the queries we'll need later to resolve collision constraints.

			this->boundsQueryTaskID = 0;
			this->worldSurfaceCollisionQueryTaskID = 0;
			this->groundSurfaceQueryTaskID = 0;

			if (this->animationMode != AnimationMode::DEATH_BY_ABYSS_FALLING)
			{
				auto boundsQuery = new Collision::ShapeInBoundsQuery();
				boundsQuery->SetShapeID(this->collisionShapeID);
				collisionSystem->MakeQuery(boundsQuery, this->boundsQueryTaskID);

				auto worldSurfaceCollisionQuery = new Collision::CollisionQuery();
				worldSurfaceCollisionQuery->SetShapeID(this->collisionShapeID);
				worldSurfaceCollisionQuery->SetUserFlagsMask(IMZADI_SHAPE_FLAG_WORLD_SURFACE);
				collisionSystem->MakeQuery(worldSurfaceCollisionQuery, this->worldSurfaceCollisionQueryTaskID);

				const Transform& objectToWorld = this->renderMesh->GetObjectToWorldTransform();
				auto groundSurfaceQuery = new Collision::RayCastQuery();
				groundSurfaceQuery->SetRay(Ray(objectToWorld.translation + Vector3(0.0, 3.0, 0.0), Vector3(0.0, -1.0, 0.0)));
				groundSurfaceQuery->SetUserFlagsMask(IMZADI_SHAPE_FLAG_WORLD_SURFACE);
				collisionSystem->MakeQuery(groundSurfaceQuery, this->groundSurfaceQueryTaskID);
			}

			break;
		}
		case TickPass::PARALLEL_WORK:
		{
			if (!this->ManageAnimation(deltaTime))
				return false;

			break;
		}
		case TickPass::RESOLVE_COLLISIONS:
		{
			this->inContactWithGround = false;

			if (this->worldSurfaceCollisionQueryTaskID)
			{
				Collision::Result* result = collisionSystem->ObtainQueryResult(this->worldSurfaceCollisionQueryTaskID);
				if (result)
				{
					auto collisionResult = dynamic_cast<Collision::CollisionQueryResult*>(result);
					if (collisionResult)
						this->HandleWorldSurfaceCollisionResult(collisionResult);

					delete result;
				}
			}

			if (this->boundsQueryTaskID)
			{
				Collision::Result* result = collisionSystem->ObtainQueryResult(this->boundsQueryTaskID);
				if (result)
				{
					auto boolResult = dynamic_cast<Collision::BoolResult*>(result);
					if (boolResult && !boolResult->GetAnswer())
						this->SetAnimationMode(AnimationMode::DEATH_BY_ABYSS_FALLING);

					delete result;
				}
			}

			if (this->groundSurfaceQueryTaskID)
			{
				Collision::Result* result = collisionSystem->ObtainQueryResult(this->groundSurfaceQueryTaskID);
				if (result)
				{
					auto rayCastResult = dynamic_cast<Collision::RayCastResult*>(result);
					if (rayCastResult)
					{
						const Collision::RayCastResult::HitData& hitData = rayCastResult->GetHitData();
						this->groundSurfacePoint = hitData.surfacePoint;
						this->groundSurfaceNormal = hitData.surfaceNormal;
					}

					delete result;
				}
			}

			if (this->inContactWithGround && !this->ConstraintVelocityWithGround())
				this->SetAnimationMode(AnimationMode::DEATH_BY_FATAL_LANDING);

			break;
		}
	}

	return true;
}

/*virtual*/ bool Biped::ManageAnimation(double deltaTime)
{
	// Make sure we're playing an appropriate animation and pump the animation system.

	auto animatedMesh = dynamic_cast<AnimatedMeshInstance*>(this->renderMesh.Get());
	if (!animatedMesh)
		return true;
	
	bool canLoop = true;
	double animationRate = 1.0;

	switch (this->animationMode)
	{
		case AnimationMode::BASIC_PLATFORMING:
		{
			const Transform& objectToWorld = this->renderMesh->GetObjectToWorldTransform();
			double heightAboveGround = (objectToWorld.translation - this->groundSurfacePoint).Length();
			if (!this->inContactWithGround && heightAboveGround > 1.0)
				animatedMesh->SetAnimation(this->GetAnimName(AnimType::JUMP));
			else
			{
				double threshold = 1.0;
				if (this->velocity.Length() < threshold)
					animatedMesh->SetAnimation(this->GetAnimName(AnimType::IDLE));
				else
				{
					animatedMesh->SetAnimation(this->GetAnimName(AnimType::RUN));

					static double conversionFactor = 0.1;
					double speed = this->velocity.Length();

					animationRate = speed * conversionFactor;
				}
			}

			break;
		}
		case AnimationMode::DEATH_BY_ABYSS_FALLING:
		{
			animatedMesh->SetAnimation(this->GetAnimName(AnimType::ABYSS_FALLING));
			canLoop = false;
			animationRate = 0.2;
			break;
		}
		case AnimationMode::DEATH_BY_FATAL_LANDING:
		{
			animatedMesh->SetAnimation(this->GetAnimName(AnimType::FATAL_LANDING));
			canLoop = false;
			animationRate = 0.2;
			break;
		}
		case AnimationMode::DEATH_BY_BADDY_HIT:
		{
			animatedMesh->SetAnimation(this->GetAnimName(AnimType::HIT_FALLING));
			canLoop = false;
			animationRate = 0.2;
			break;
		}
	}

	if (!animatedMesh->AdvanceAnimation(deltaTime * animationRate, canLoop))
	{
		if (this->animationMode == AnimationMode::DEATH_BY_FATAL_LANDING ||
			this->animationMode == AnimationMode::DEATH_BY_ABYSS_FALLING ||
			this->animationMode == AnimationMode::DEATH_BY_BADDY_HIT)
		{
			if (!this->OnBipedDied())
				return false;
		}
	}

	return true;
}

/*virtual*/ bool Biped::OnBipedDied()
{
	if (this->canRestart)
	{
		this->Reset();
		return true;
	}

	return false;
}

/*virtual*/ void Biped::OnBipedFatalLanding()
{
}

/*virtual*/ void Biped::OnBipedBaddyHit()
{
}

/*virtual*/ void Biped::OnBipedAbyssFalling()
{
}

void Biped::SetAnimationMode(AnimationMode newMode)
{
	if (this->animationMode == newMode)
		return;

	this->animationMode = newMode;

	switch (this->animationMode)
	{
		case AnimationMode::DEATH_BY_FATAL_LANDING:
		{
			this->OnBipedFatalLanding();
			break;
		}
		case AnimationMode::DEATH_BY_ABYSS_FALLING:
		{
			this->OnBipedAbyssFalling();
			break;
		}
		case AnimationMode::DEATH_BY_BADDY_HIT:
		{
			this->OnBipedBaddyHit();
			break;
		}
	}
}

/*virtual*/ bool Biped::ConstraintVelocityWithGround()
{
	this->velocity.y = 0.0;		// TODO: Maybe reject from ground surface normal?
	return true;
}

void Biped::SetRestartLocation(const Vector3& restartLocation)
{
	this->restartTransformObjectToWorld.translation = restartLocation;
}

void Biped::SetRestartOrientation(const Quaternion& restartOrientation)
{
	this->restartTransformObjectToWorld.matrix.SetFromQuat(restartOrientation);
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

void Biped::HandleWorldSurfaceCollisionResult(Collision::CollisionQueryResult* collisionResult)
{
	if (collisionResult->GetCollisionStatusArray().size() == 0)
		return;

	Vector3 averageSeperationDelta(0.0, 0.0, 0.0);
	Collision::ShapeID newGroundShapeID = 0;
	Transform newGroundObjectToWorld;
	newGroundObjectToWorld.SetIdentity();
	this->continuouslyUpdatePlatformTransform = false;

	for (const auto& collisionStatus : collisionResult->GetCollisionStatusArray())
	{
		Collision::ShapeID otherShapeID = collisionStatus->GetOtherShape(this->collisionShapeID);
		
		Vector3 separationDelta = collisionStatus->GetSeparationDelta(this->collisionShapeID);
		averageSeperationDelta += separationDelta;

		Vector3 upVector(0.0, 1.0, 0.0);
		double angle = upVector.AngleBetween(separationDelta.Normalized());
		if (angle < M_PI / 3.0)
		{
			this->inContactWithGround = true;
			newGroundShapeID = otherShapeID;
			const Collision::Shape* shape = collisionStatus->GetShape(otherShapeID);
			if ((shape->GetUserFlags() & IMZADI_SHAPE_FLAG_NON_RELATIVE) == 0)
			{
				newGroundObjectToWorld = shape->GetObjectToWorldTransform();
				this->continuouslyUpdatePlatformTransform = true;
			}
		}
	}

	averageSeperationDelta /= double(collisionResult->GetCollisionStatusArray().size());

	Transform objectToWorld = this->platformToWorld * this->objectToPlatform;

	this->groundShapeID = newGroundShapeID;
	this->platformToWorld = newGroundObjectToWorld;

	Transform worldToPlatform;
	worldToPlatform.Invert(this->platformToWorld);
	this->objectToPlatform = worldToPlatform * objectToWorld;
	objectToWorld = this->platformToWorld * this->objectToPlatform;

	if (averageSeperationDelta.Length() > 0.0)
	{
		objectToWorld.translation += averageSeperationDelta;
		this->objectToPlatform = worldToPlatform * objectToWorld;
	}
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

	if (this->collisionShapeID)
	{
		Collision::System* collisionSystem = Game::Get()->GetCollisionSystem();

		// Make sure that the collision shape transform for the biped matches the biped's render mesh transform.
		auto command = new Collision::ObjectToWorldCommand();
		command->SetShapeID(this->collisionShapeID);
		command->objectToWorld = transform;
		collisionSystem->IssueCommand(command);
	}

	return true;
}

/*virtual*/ void Biped::Reset()
{
	this->velocity.SetComponents(0.0, 0.0, 0.0);
	this->mass = 1.0;
	this->platformToWorld.SetIdentity();
	this->objectToPlatform = this->restartTransformObjectToWorld;
	this->inContactWithGround = false;
	this->groundShapeID = 0;
	this->SetAnimationMode(AnimationMode::BASIC_PLATFORMING);

	auto animatedMesh = dynamic_cast<AnimatedMeshInstance*>(this->renderMesh.Get());
	if (animatedMesh)
		animatedMesh->ClearTransition();

	Game::Get()->GetEventSystem()->SendEvent("Biped", new BipedResetEvent(this->GetHandle()));
}