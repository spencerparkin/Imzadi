#include "TriggerBox.h"
#include "Log.h"
#include "Collision/System.h"
#include "Collision/Shapes/Box.h"
#include "Collision/Query.h"
#include "Collision/CollisionCache.h"

using namespace Imzadi;

TriggerBox::TriggerBox()
{
	this->collisionShapeID = 0;
	this->collisionQueryTaskID = 0;
}

/*virtual*/ TriggerBox::~TriggerBox()
{
}

/*virtual*/ bool TriggerBox::Setup()
{
	if (!Entity::Setup())
		return false;

	if (!this->data.Get())
	{
		IMZADI_LOG_ERROR("No data set on trigger box.");
		return false;
	}

	this->SetName(this->data->GetName());

	const AxisAlignedBoundingBox& box = this->data->GetBox();

	Vector3 extents = (box.maxCorner - box.minCorner) / 2.0;
	Vector3 center = (box.maxCorner + box.minCorner) / 2.0;

	CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

	Transform objectToWorld;
	objectToWorld.matrix.SetIdentity();
	objectToWorld.translation = center;

	auto boxShape = BoxShape::Create();
	boxShape->SetExtents(extents);
	boxShape->SetObjectToWorldTransform(objectToWorld);
	boxShape->SetUserFlags(IMZADI_SHAPE_FLAG_TRIGGER_BOX);
	this->collisionShapeID = collisionSystem->AddShape(boxShape, 0);

	return true;
}

/*virtual*/ bool TriggerBox::Shutdown()
{
	Entity::Shutdown();

	// TODO: Do we need to issue a command to the collision system here
	//       to remove our collision shape?

	return true;
}

/*virtual*/ bool TriggerBox::Tick(TickPass tickPass, double deltaTime)
{
	if (!Entity::Tick(tickPass, deltaTime))
		return false;

	CollisionSystem* collisionSystem = Game::Get()->GetCollisionSystem();

	switch (tickPass)
	{
		case TickPass::SUBMIT_COLLISION_QUERIES:
		{
			auto collisionQuery = CollisionQuery::Create();
			collisionQuery->SetShapeID(this->collisionShapeID);
			collisionSystem->MakeQuery(collisionQuery, this->collisionQueryTaskID);

			break;
		}
		case TickPass::RESOLVE_COLLISIONS:
		{
			if (this->collisionQueryTaskID)
			{
				Result* result = collisionSystem->ObtainQueryResult(this->collisionQueryTaskID);
				if (result)
				{
					auto collisionResult = dynamic_cast<CollisionQueryResult*>(result);
					if (collisionResult)
						this->UpdateCollisionState(collisionResult);

					collisionSystem->Free(result);
				}
			}

			break;
		}
	}

	return true;
}

void TriggerBox::UpdateCollisionState(CollisionQueryResult* collisionResult)
{
	const std::vector<Reference<ShapePairCollisionStatus>>& collisionStatusArray = collisionResult->GetCollisionStatusArray();
	for (const auto& collisionStatus : collisionStatusArray)
	{
		ShapeID shapeID = collisionStatus->GetOtherShape(this->collisionShapeID);

		if (this->shapeSet.find(shapeID) == this->shapeSet.end())
		{
			this->shapeSet.insert(shapeID);
			Game::Get()->GetEventSystem()->SendEvent(this->data->GetEventChannelName(), new TriggerBoxEvent(TriggerBoxEvent::Type::SHAPE_ENTERED, shapeID, this->GetName()));
		}
	}

	std::vector<ShapeID> shapesToRemoveArray;
	for (ShapeID shapeID : this->shapeSet)
	{
		bool found = false;
		for (const auto& collisionStatus : collisionStatusArray)
		{
			if (collisionStatus->GetOtherShape(this->collisionShapeID) == shapeID)
			{
				found = true;
				break;
			}
		}

		if (!found)
			shapesToRemoveArray.push_back(shapeID);
	}

	for (ShapeID shapeID : shapesToRemoveArray)
	{
		this->shapeSet.erase(shapeID);
		Game::Get()->GetEventSystem()->SendEvent(this->data->GetEventChannelName(), new TriggerBoxEvent(TriggerBoxEvent::Type::SHAPE_EXITED, shapeID, this->GetName()));
	}
}