#include "Query.h"
#include "Result.h"
#include "Thread.h"
#include "Error.h"
#include "BoundingBoxTree.h"
#include <format>

using namespace Collision;

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
	COLL_SYS_ASSERT(result != nullptr);
	thread->StoreResult(result, this->GetTaskID());
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
	auto renderResult = DebugRenderResult::Create();
	thread->DebugVisualize(renderResult, this->drawFlags);
	return renderResult;
}

/*static*/ DebugRenderQuery* DebugRenderQuery::Create()
{
	return new DebugRenderQuery();
}

//--------------------------------- RayCastQuery ---------------------------------

RayCastQuery::RayCastQuery()
{
}

/*virtual*/ RayCastQuery::~RayCastQuery()
{
}

/*virtual*/ Result* RayCastQuery::ExecuteQuery(Thread* thread)
{
	const BoundingBoxTree& boxTree = thread->GetBoundingBoxTree();
	RayCastResult* result = RayCastResult::Create();
	boxTree.RayCast(this->GetRay(), result);
	return result;
}

/*static*/ RayCastQuery* RayCastQuery::Create()
{
	return new RayCastQuery();
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
	Shape* shape = thread->FindShape(this->shapeID);
	if (!shape)
	{
		auto errorResult = ErrorResult::Create();
		errorResult->SetErrorMessage(std::format("Failed to find a shape with ID {}.", this->shapeID));
		return errorResult;
	}

	auto result = TransformResult::Create();
	result->transform = shape->GetObjectToWorldTransform();
	return result;
}

//--------------------------------- CollisionQuery ---------------------------------

CollisionQuery::CollisionQuery()
{
}

/*virtual*/ CollisionQuery::~CollisionQuery()
{
}

/*virtual*/ Result* CollisionQuery::ExecuteQuery(Thread* thread)
{
	// TODO: Write this.
	// TODO: Note that if the underlying system is smart, work to see if A collides with B will not be duplicated to find that B collides with A.
	return nullptr;
}

/*static*/ CollisionQuery* CollisionQuery::Create()
{
	return new CollisionQuery();
}