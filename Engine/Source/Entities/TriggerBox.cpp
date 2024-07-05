#include "TriggerBox.h"
#include "Log.h"
#include "Collision/System.h"
#include "Collision/Shapes/Box.h"
#include "Collision/Query.h"
#include "Collision/Result.h"

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
		case TickPass::QUERY_TICK:
		{
			auto collisionQuery = CollisionQuery::Create();
			collisionQuery->SetShapeID(this->collisionShapeID);
			collisionSystem->MakeQuery(collisionQuery, this->collisionQueryTaskID);

			break;
		}
		case TickPass::RESULT_TICK:
		{
			if (this->collisionQueryTaskID)
			{
				Result* result = collisionSystem->ObtainQueryResult(this->collisionQueryTaskID);
				if (result)
				{
					// TODO: Process collision here.  Send on-enter or on-leave events as necessary.

					collisionSystem->Free(result);
				}
			}

			break;
		}
	}

	return true;
}