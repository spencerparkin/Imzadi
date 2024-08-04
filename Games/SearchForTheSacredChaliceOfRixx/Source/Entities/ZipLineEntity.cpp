#include "ZipLineEntity.h"
#include "GameApp.h"
#include "Characters/DeannaTroi.h"
#include "Collision/Shapes/Sphere.h"
#include "Collision/CollisionCache.h"
#include "Log.h"

ZipLineEntity::ZipLineEntity()
{
	for (int i = 0; i < 2; i++)
	{
		this->collisionQueryTaskID[i] = 0;
		this->sphereShapeID[i] = 0;
	}
}

/*virtual*/ ZipLineEntity::~ZipLineEntity()
{
}

void ZipLineEntity::SetZipLine(ZipLine* givenZipLine)
{
	this->zipLine = givenZipLine;
}

/*virtual*/ bool ZipLineEntity::Setup()
{
	if (!this->zipLine)
		return false;

	Imzadi::CollisionSystem* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();
	if (!collisionSystem)
		return false;

	for (int i = 0; i < 2; i++)
	{
		Imzadi::Transform objectToWorld;
		objectToWorld.SetIdentity();
		objectToWorld.translation = this->zipLine->GetLineSegment().point[i];

		auto sphereShape = Imzadi::SphereShape::Create();
		sphereShape->SetObjectToWorldTransform(objectToWorld);
		sphereShape->SetCenter(Imzadi::Vector3(0.0, 0.0, 0.0));
		sphereShape->SetRadius(this->zipLine->GetRadius());
		this->sphereShapeID[i] = collisionSystem->AddShape(sphereShape, 0);
	}

	return true;
}

/*virtual*/ bool ZipLineEntity::Shutdown()
{
	Imzadi::CollisionSystem* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

	for (int i = 0; i < 2; i++)
	{
		collisionSystem->RemoveShape(this->sphereShapeID[i]);
		this->sphereShapeID[i] = 0;
	}

	return true;
}

/*virtual*/ bool ZipLineEntity::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	switch (tickPass)
	{
		case Imzadi::TickPass::SUBMIT_COLLISION_QUERIES:
		{
			Imzadi::CollisionSystem* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			for (int i = 0; i < 2; i++)
			{
				auto query = Imzadi::CollisionQuery::Create();
				query->SetShapeID(this->sphereShapeID[i]);
				query->SetUserFlagsMask(IMZADI_SHAPE_FLAG_BIPED_ENTITY);
				collisionSystem->MakeQuery(query, this->collisionQueryTaskID[i]);
			}

			break;
		}
		case Imzadi::TickPass::RESOLVE_COLLISIONS:
		{
			Imzadi::CollisionSystem* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			for (int i = 0; i < 2; i++)
			{
				if (!this->collisionQueryTaskID[i])
					continue;

				Imzadi::Result* result = collisionSystem->ObtainQueryResult(this->collisionQueryTaskID[i]);
				if (result)
				{
					auto collisionResult = dynamic_cast<Imzadi::CollisionQueryResult*>(result);
					if (collisionResult)
						this->HandleCollisionResult(collisionResult, i);

					collisionSystem->Free<Imzadi::Result>(result);
				}
			}

			break;
		}
	}

	return true;
}

void ZipLineEntity::HandleCollisionResult(Imzadi::CollisionQueryResult* collisionResult, int i)
{
	for (const Imzadi::Reference<Imzadi::ShapePairCollisionStatus>& collisionPair : collisionResult->GetCollisionStatusArray())
	{
		Imzadi::ShapeID shapeID = collisionPair->GetOtherShape(this->sphereShapeID[i]);
		Imzadi::Reference<Imzadi::Entity> foundEntity;
		if (!Imzadi::Game::Get()->FindEntityByShapeID(shapeID, foundEntity))
			continue;

		IMZADI_LOG_INFO("Zip line point %d hit by %s.", i, foundEntity->GetName().c_str());

		// TODO: Recall that the zip-line force is the component of the gravity force in the direction of the zip-line.
	}
}