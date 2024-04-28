#include "Query.h"
#include "Result.h"
#include "Thread.h"
#include "BoundingBoxTree.h"

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
	if (result)
		thread->StoreResult(result, this->GetTaskID());
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

	std::vector<const Shape*> shapeArray;
	boxTree.RayCast(ray, shapeArray);

	RayCastResult::HitData hitData;
	hitData.shapeID = 0;

	if (shapeArray.size() > 0)
	{
		double smallestAlpha = std::numeric_limits<double>::max();
		for (const Shape* shape : shapeArray)
		{
			double alpha = 0.0;
			Vector3 unitSurfaceNormal;
			if (shape->RayCast(ray, alpha, unitSurfaceNormal) && alpha < smallestAlpha)
			{
				smallestAlpha = alpha;
				hitData.shapeID = shape->GetShapeID();
				hitData.surfaceNormal = unitSurfaceNormal;
				hitData.surfacePoint = ray.CalculatePoint(alpha);
			}
		}
	}

	RayCastResult* result = RayCastResult::Create();
	result->SetHitData(hitData);
	return result;
}

/*static*/ RayCastQuery* RayCastQuery::Create()
{
	return new RayCastQuery();
}