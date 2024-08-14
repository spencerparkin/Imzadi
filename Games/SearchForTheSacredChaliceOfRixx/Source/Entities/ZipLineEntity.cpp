#include "ZipLineEntity.h"
#include "GameApp.h"
#include "Characters/DeannaTroi.h"
#include "Collision/Shapes/Sphere.h"
#include "Collision/CollisionCache.h"
#include "Entities/ZipLineRider.h"
#include "Log.h"

ZipLineEntity::ZipLineEntity()
{
	this->collisionQueryTaskID = 0;
	this->sphereShapeID = 0;
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

	Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();
	if (!collisionSystem)
		return false;

	Imzadi::Transform objectToWorld;
	objectToWorld.SetIdentity();
	objectToWorld.translation = this->zipLine->GetLineSegment().point[0];

	auto sphereShape = new Imzadi::Collision::SphereShape();
	sphereShape->SetObjectToWorldTransform(objectToWorld);
	sphereShape->SetCenter(Imzadi::Vector3(0.0, 0.0, 0.0));
	sphereShape->SetRadius(this->zipLine->GetRadius());
	this->sphereShapeID = collisionSystem->AddShape(sphereShape, 0);

	return true;
}

/*virtual*/ bool ZipLineEntity::Shutdown()
{
	Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

	collisionSystem->RemoveShape(this->sphereShapeID);
	this->sphereShapeID = 0;

	return true;
}

/*virtual*/ bool ZipLineEntity::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	switch (tickPass)
	{
		case Imzadi::TickPass::SUBMIT_COLLISION_QUERIES:
		{
			Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

			auto query = new Imzadi::Collision::CollisionQuery();
			query->SetShapeID(this->sphereShapeID);
			query->SetUserFlagsMask(IMZADI_SHAPE_FLAG_BIPED_ENTITY);
			collisionSystem->MakeQuery(query, this->collisionQueryTaskID);

			break;
		}
		case Imzadi::TickPass::RESOLVE_COLLISIONS:
		{
			if (this->collisionQueryTaskID)
			{
				Imzadi::Collision::System* collisionSystem = Imzadi::Game::Get()->GetCollisionSystem();

				Imzadi::Collision::Result* result = collisionSystem->ObtainQueryResult(this->collisionQueryTaskID);
				if (result)
				{
					auto collisionResult = dynamic_cast<Imzadi::Collision::CollisionQueryResult*>(result);
					if (collisionResult)
						this->HandleCollisionResult(collisionResult);

					delete result;
				}
			}

			break;
		}
	}

	return true;
}

void ZipLineEntity::HandleCollisionResult(Imzadi::Collision::CollisionQueryResult* collisionResult)
{
	for (const Imzadi::Reference<Imzadi::Collision::ShapePairCollisionStatus>& collisionPair : collisionResult->GetCollisionStatusArray())
	{
		Imzadi::Collision::ShapeID shapeID = collisionPair->GetOtherShape(this->sphereShapeID);
		Imzadi::Reference<Imzadi::Entity> foundEntity;
		if (!Imzadi::Game::Get()->FindEntityByShapeID(shapeID, foundEntity))
			continue;

		auto character = dynamic_cast<Character*>(foundEntity.Get());

		if (!this->FindZipLineRider(character) && !character->IsInContactWithGround() && character->HangingOnToZipLine())
		{
			auto rider = Imzadi::Game::Get()->SpawnEntity<ZipLineRider>();
			rider->SetZipLine(this->zipLine);
			rider->SetCharacter(character);
		}
	}
}

ZipLineRider* ZipLineEntity::FindZipLineRider(Character* character)
{
	ZipLineRider* foundZipLineRider = nullptr;

	std::vector<ZipLineRider*> zipLineRiderArray;
	Imzadi::Game::Get()->CollectEntities<ZipLineRider>(zipLineRiderArray);
		
	for (int i = 0; i < (signed)zipLineRiderArray.size(); i++)
	{
		ZipLineRider* zipLineRider = zipLineRiderArray[i];
		if (zipLineRider->GetCharacter() == character)
		{
			foundZipLineRider = zipLineRider;
			break;
		}
	}

	return foundZipLineRider;
}