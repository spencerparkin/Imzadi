#include "Borggy.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "GameApp.h"
#include "Math/Ray.h"
#include "Collision/Query.h"
#include "Collision/Result.h"
#include "Audio/System.h"

Borggy::Borggy()
{
	this->SetName("Borggy");
	this->disposition = Disposition::MEANDERING;
	this->meanderState = MeanderState::UNKNOWN;
	this->rayCastQueryTaskID = 0;
	this->rayCastAttackQueryTaskID = 0;
	this->rotationTimeRemainding = 0.0;
	this->meanderingRotationRate = 1.0;
	this->meanderingMoveSpeed = 10.0;
	this->attackMoveSpeed = 20.0;
	this->assimulatedHuman = false;
	this->canRestart = false;
}

/*virtual*/ Borggy::~Borggy()
{
}

/*virtual*/ bool Borggy::Setup()
{
	std::string modelFile = "Models/Borggy/Borggy.skinned_render_mesh";
	this->renderMesh.SafeSet(Imzadi::Game::Get()->LoadAndPlaceRenderMesh(modelFile));

	if (!Character::Setup())
		return false;

	return true;
}

/*virtual*/ bool Borggy::Shutdown()
{
	Character::Shutdown();

	return true;
}

/*virtual*/ void Borggy::ConfigureCollisionCapsule(Imzadi::Collision::CapsuleShape* capsule)
{
	capsule->SetVertex(0, Imzadi::Vector3(0.0, 2.5, 0.0));
	capsule->SetVertex(1, Imzadi::Vector3(0.0, 6.5, 0.0));
	capsule->SetRadius(2.5);
	capsule->SetUserFlags(IMZADI_SHAPE_FLAG_BIPED_ENTITY | SHAPE_FLAG_TALKER | SHAPE_FLAG_BADDY);
}

/*virtual*/ bool Borggy::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	if (!Character::Tick(tickPass, deltaTime))
		return false;

	switch (tickPass)
	{
		case Imzadi::TickPass::SUBMIT_COLLISION_QUERIES:
		{
			Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			Imzadi::Transform objectToWorld;
			this->GetTransform(objectToWorld);

			Imzadi::Vector3 xAxis, yAxis, zAxis;
			objectToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

			Imzadi::Ray ray;
			ray.unitDirection = (-yAxis - 0.5 * zAxis).Normalized();
			ray.origin = objectToWorld.translation + 2.0 * yAxis - 4.0 * zAxis;

			Imzadi::Vector3 boxExtent(8.0, 8.0, 8.0);
			Imzadi::AxisAlignedBoundingBox boundingBox;
			boundingBox.MakeReadyForExpansion();
			boundingBox.Expand(objectToWorld.translation + boxExtent);
			boundingBox.Expand(objectToWorld.translation - boxExtent);

			auto rayCastQuery = new Imzadi::Collision::RayCastQuery();
			rayCastQuery->SetRay(ray);
			rayCastQuery->SetBoundingBox(boundingBox);
			rayCastQuery->SetUserFlagsMask(IMZADI_SHAPE_FLAG_WORLD_SURFACE);
			collisionSystem->MakeQuery(rayCastQuery, this->rayCastQueryTaskID);
#if 0
			Imzadi::DebugLines* debugLines = Imzadi::Game::Get()->GetDebugLines();
			Imzadi::DebugLines::Line line;
			line.color.SetComponents(1.0, 0.0, 0.0);
			line.segment.point[0] = ray.origin;
			line.segment.point[1] = ray.origin + ray.unitDirection * 100.0;
			debugLines->AddLine(line);
#endif
			break;
		}
		case Imzadi::TickPass::RESOLVE_COLLISIONS:
		{
			this->HandlePlatformRayCast(deltaTime);
			break;
		}
	}

	if (this->disposition == Disposition::MEANDERING)
	{
		switch (tickPass)
		{
			case Imzadi::TickPass::SUBMIT_COLLISION_QUERIES:
			{
				Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

				Imzadi::Transform objectToWorld;
				this->GetTransform(objectToWorld);

				Imzadi::Vector3 xAxis, yAxis, zAxis;
				objectToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);
				
				Imzadi::Ray ray;
				ray.unitDirection = -zAxis;
				ray.origin = objectToWorld.translation + 4.0 * yAxis - 3.0 * zAxis;

				Imzadi::Vector3 boxExtent(20.0, 20.0, 20.0);
				Imzadi::AxisAlignedBoundingBox boundingBox;
				boundingBox.MakeReadyForExpansion();
				boundingBox.Expand(objectToWorld.translation + boxExtent);
				boundingBox.Expand(objectToWorld.translation - boxExtent);

				auto rayCastAttackQuery = new Imzadi::Collision::RayCastQuery();
				rayCastAttackQuery->SetRay(ray);
				rayCastAttackQuery->SetBoundingBox(boundingBox);
				rayCastAttackQuery->SetUserFlagsMask(IMZADI_SHAPE_FLAG_BIPED_ENTITY);
				collisionSystem->MakeQuery(rayCastAttackQuery, this->rayCastAttackQueryTaskID);

				break;
			}
			case Imzadi::TickPass::RESOLVE_COLLISIONS:
			{
				this->HandleAttackRayCast();
				break;
			}
		}
	}

	return true;
}

/*virtual*/ void Borggy::IntegrateVelocity(const Imzadi::Vector3& acceleration, double deltaTime)
{
	switch (this->disposition)
	{
		case Disposition::MEANDERING:
		{
			if (this->inContactWithGround)
			{
				if (this->meanderState != MeanderState::MOVE_FORWARD_UNTIL_BORDER_HIT)
					this->velocity.SetComponents(0.0, 0.0, 0.0);
				else
				{
					Imzadi::Vector3 xAxis, yAxis, zAxis;
					this->objectToPlatform.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

					this->velocity = -zAxis * this->meanderingMoveSpeed;
				}
			}

			break;
		}
	}

	Character::IntegrateVelocity(acceleration, deltaTime);
}

/*virtual*/ void Borggy::AdjustFacingDirection(double deltaTime)
{
	if (this->inContactWithGround && this->disposition == Disposition::MEANDERING)
	{
		switch (this->meanderState)
		{
			case MeanderState::ROTATE_LEFT_UNTIL_READY:
			case MeanderState::ROTATE_RIGHT_UNTIL_READY:
			{
				double rotationAngle = this->meanderingRotationRate * deltaTime * (this->meanderState == MeanderState::ROTATE_LEFT_UNTIL_READY ? -1.0 : 1.0);

				Imzadi::Matrix3x3 rotation;
				rotation.SetFromAxisAngle(Imzadi::Vector3(0.0, 1.0, 0.0), rotationAngle);

				this->objectToPlatform.matrix = (rotation * this->objectToPlatform.matrix).Orthonormalized(IMZADI_AXIS_FLAG_X);

				break;
			}
		}
	}
	else
	{
		Character::AdjustFacingDirection(deltaTime);
	}
}

void Borggy::HandlePlatformRayCast(double deltaTime)
{
	Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

	if (this->rayCastQueryTaskID == 0)
		return;
	
	std::unique_ptr<Imzadi::Collision::Result> result(collisionSystem->ObtainQueryResult(this->rayCastQueryTaskID));
	if (!result)
		return;
	
	auto rayCastResult = dynamic_cast<Imzadi::Collision::RayCastResult*>(result.get());
	if (!rayCastResult)
		return;
	
	bool foundCliffEdgeOrWall = false;
	const Imzadi::Collision::RayCastResult::HitData& hitData = rayCastResult->GetHitData();

	bool foundCliffEdge = false;
	bool foundWall = false;
	if (hitData.shapeID == 0)
		foundCliffEdge = true;
	else
	{
		double angle = hitData.surfaceNormal.AngleBetween(Imzadi::Vector3(0.0, 1.0, 0.0));
		if (angle > M_PI / 4.0)
			foundWall = true;
	}

	if (this->disposition == Disposition::ATTACKING)
	{
		if (foundWall)
			this->disposition = Disposition::MEANDERING;
		else
			return;
	}

	if (foundCliffEdge || foundWall)
	{
		switch (this->meanderState)
		{
			case MeanderState::UNKNOWN:
			case MeanderState::MOVE_FORWARD_UNTIL_BORDER_HIT:
			{
				this->meanderState = this->random.CoinFlip() ? MeanderState::ROTATE_LEFT_UNTIL_READY : MeanderState::ROTATE_RIGHT_UNTIL_READY;
				this->rotationTimeRemainding = this->random.InRange(0.5, 4.5);
				break;
			}
		}
	}
	else
	{
		switch (this->meanderState)
		{
			case MeanderState::UNKNOWN:
			{
				this->meanderState = MeanderState::MOVE_FORWARD_UNTIL_BORDER_HIT;
				break;
			}
			case MeanderState::ROTATE_LEFT_UNTIL_READY:
			case MeanderState::ROTATE_RIGHT_UNTIL_READY:
			{
				this->rotationTimeRemainding -= deltaTime;
				if (this->rotationTimeRemainding <= 0.0)
					this->meanderState = MOVE_FORWARD_UNTIL_BORDER_HIT;
				break;
			}
		}
	}
}

void Borggy::HandleAttackRayCast()
{
	Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

	if (this->rayCastAttackQueryTaskID == 0)
		return;

	std::unique_ptr<Imzadi::Collision::Result> result(collisionSystem->ObtainQueryResult(this->rayCastAttackQueryTaskID));
	if (!result)
		return;

	auto rayCastAttackResult = dynamic_cast<Imzadi::Collision::RayCastResult*>(result.get());
	if (!rayCastAttackResult)
		return;

	const Imzadi::Collision::RayCastResult::HitData& hitData = rayCastAttackResult->GetHitData();
	if (hitData.shapeID == 0)
		return;

	Imzadi::Reference<Imzadi::Entity> foundEntity;
	if (!Imzadi::Game::Get()->FindEntityByShapeID(hitData.shapeID, foundEntity))
		return;

	if (foundEntity->GetName() != "Alice")
		return;

	this->disposition = Disposition::ATTACKING;

	Imzadi::Transform aliceObjectToWorld;
	foundEntity->GetTransform(aliceObjectToWorld);

	Imzadi::Transform borgObjectToWorld;
	this->GetTransform(borgObjectToWorld);

	this->velocity = (aliceObjectToWorld.translation - borgObjectToWorld.translation).Normalized() * this->attackMoveSpeed;

	Imzadi::Transform worldToPlatform;
	worldToPlatform.Invert(this->platformToWorld);
	this->velocity = worldToPlatform.TransformVector(this->velocity);

	Imzadi::Game::Get()->GetAudioSystem()->PlaySound("ResistanceIsFutile");
}

/*virtual*/ void Borggy::OnBipedAbyssFalling()
{
	if (!this->assimulatedHuman)
		Imzadi::Game::Get()->GetAudioSystem()->PlaySound("ResistanceIsNotFutile");
}

/*virtual*/ std::string Borggy::GetAnimName(Imzadi::Biped::AnimType animType)
{
	switch (animType)
	{
	case Imzadi::Biped::AnimType::IDLE:
		return "BorggyIdle";
	case Imzadi::Biped::AnimType::RUN:
		return "BorggyRun";
	}

	return "";
}