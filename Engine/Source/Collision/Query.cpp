#include "Query.h"
#include "Result.h"
#include "Thread.h"
#include "BoundingBoxTree.h"
#include "Log.h"
#include <format>

using namespace Imzadi;
using namespace Imzadi::Collision;

//--------------------------------- Query ---------------------------------

Query::Query()
{
}

/*virtual*/ Query::~Query()
{
}

/*virtual*/ void Query::Execute(Thread* thread)
{
	Result* result = this->ExecuteQuery(thread);
	if (result)
		thread->StoreResult(result, this->GetTaskID());
}

//--------------------------------- ShapeQuery ---------------------------------

StatsQuery::StatsQuery()
{
}

/*virtual*/ StatsQuery::~StatsQuery()
{
}

/*virtual*/ Result* StatsQuery::ExecuteQuery(Thread* thread)
{
	auto result = new StatsResult();
	thread->GatherStats(result);
	return result;
}

//--------------------------------- ShapeQuery ---------------------------------

ShapeQuery::ShapeQuery()
{
	this->shapeID = 0;
}

/*virtual*/ ShapeQuery::~ShapeQuery()
{
}

//--------------------------------- DebugRenderQuery ---------------------------------

DebugRenderQuery::DebugRenderQuery()
{
	this->drawFlags = 0;
}

/*virtual*/ DebugRenderQuery::~DebugRenderQuery()
{
}

/*virtual*/ Result* DebugRenderQuery::ExecuteQuery(Thread* thread)
{
	IMZADI_COLLISION_PROFILE("Debug Render Query");
	auto renderResult = new DebugRenderResult();
	thread->DebugVisualize(renderResult, this->drawFlags);
	return renderResult;
}

//--------------------------------- RayCastQuery ---------------------------------

RayCastQuery::RayCastQuery()
{
	this->userFlagsMask = 0xFFFFFFFFFFFFFFFF;
	this->boundingBox.MakeReadyForExpansion();
}

/*virtual*/ RayCastQuery::~RayCastQuery()
{
}

/*virtual*/ Result* RayCastQuery::ExecuteQuery(Thread* thread)
{
	// TODO: My profiling has revealed that this is where the slowness is coming from.
	//       Optimize this.  I think that one obvious optimization here is to limit the
	//       length of ray-casts.  In any case, the infinite-length ray-cast is probably
	//       written wrong, and I should start by trying to fix it.
	IMZADI_COLLISION_PROFILE("Ray Cast Query");
	const BoundingBoxTree& boxTree = thread->GetBoundingBoxTree();
	auto result = new RayCastResult();
	boxTree.RayCast(this->ray, this->boundingBox, this->userFlagsMask, result);
	return result;
}

//--------------------------------- ObjectToWorldQuery ---------------------------------

ObjectToWorldQuery::ObjectToWorldQuery()
{
}

/*virtual*/ ObjectToWorldQuery::~ObjectToWorldQuery()
{
}

/*virtual*/ Result* ObjectToWorldQuery::ExecuteQuery(Thread* thread)
{
	IMZADI_COLLISION_PROFILE("Object-to-World Query");

	Shape* shape = thread->FindShape(this->shapeID);
	if (!shape)
	{
		IMZADI_LOG_ERROR(std::format("Failed to find a shape with ID {}.", this->shapeID));
		return nullptr;
	}

	auto result = new ObjectToWorldResult();
	result->objectToWorld = shape->GetObjectToWorldTransform();
	return result;
}

//--------------------------------- CollisionQuery ---------------------------------

CollisionQuery::CollisionQuery()
{
	this->userFlagsMask = 0xFFFFFFFFFFFFFFFF;
}

/*virtual*/ CollisionQuery::~CollisionQuery()
{
}

/*virtual*/ Result* CollisionQuery::ExecuteQuery(Thread* thread)
{
	IMZADI_COLLISION_PROFILE("Collision Query");

	Shape* shape = thread->FindShape(this->shapeID);
	if (!shape)
	{
		IMZADI_LOG_ERROR(std::format("Failed to find a shape with ID {}.", this->shapeID));
		return nullptr;
	}
	
	auto collisionResult = new CollisionQueryResult();
	collisionResult->SetShapeID(shape->GetShapeID());
	collisionResult->SetObjectToWorldTransform(shape->GetObjectToWorldTransform());

	BoundingBoxTree& tree = thread->GetBoundingBoxTree();
	if (!tree.CalculateCollision(shape, this->userFlagsMask, collisionResult))
	{
		delete collisionResult;

		IMZADI_LOG_ERROR(std::format("Failed to calculate collision result for shape with ID {}.", this->shapeID));
		return nullptr;
	}

	return collisionResult;
}

//--------------------------------- ShapeInBoundsQuery ---------------------------------

ShapeInBoundsQuery::ShapeInBoundsQuery()
{
}

/*virtual*/ ShapeInBoundsQuery::~ShapeInBoundsQuery()
{
}

/*virtual*/ Result* ShapeInBoundsQuery::ExecuteQuery(Thread* thread)
{
	IMZADI_COLLISION_PROFILE("Shape-in-Bounds Query");

	auto result = new BoolResult();
	result->SetAnswer(false);

	BoundingBoxTree& tree = thread->GetBoundingBoxTree();
	Shape* shape = tree.FindShape(this->GetShapeID());
	if (shape && shape->IsBound())
		result->SetAnswer(true);

	return result;
}

//--------------------------------- ProfileStatsQuery ---------------------------------

ProfileStatsQuery::ProfileStatsQuery()
{
}

/*virtual*/ ProfileStatsQuery::~ProfileStatsQuery()
{
}

/*virtual*/ Result* ProfileStatsQuery::ExecuteQuery(Thread* thread)
{
	auto result = new StringResult();
	result->SetText(collisionProfileData.PrintStats());
	return result;
}