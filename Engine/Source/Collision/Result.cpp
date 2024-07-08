#include "Result.h"
#include "CollisionCache.h"
#include "Math/AxisAlignedBoundingBox.h"

using namespace Imzadi;

//-------------------------------- Result --------------------------------

Result::Result()
{
}

/*virtual*/ Result::~Result()
{
}

/*static*/ void Result::Free(Result* result)
{
	delete result;
}

//-------------------------------- BoolResult --------------------------------

BoolResult::BoolResult()
{
	this->answer = false;
}

/*virtual*/ BoolResult::~BoolResult()
{
}

/*static*/ BoolResult* BoolResult::Create()
{
	return new BoolResult();
}

//-------------------------------- DebugRenderResult --------------------------------

DebugRenderResult::DebugRenderResult()
{
	this->renderLineArray = new std::vector<RenderLine>();
}

/*virtual*/ DebugRenderResult::~DebugRenderResult()
{
	delete this->renderLineArray;
}

void DebugRenderResult::AddRenderLine(const RenderLine& renderLine)
{
	this->renderLineArray->push_back(renderLine);
}

/*static*/ DebugRenderResult* DebugRenderResult::Create()
{
	return new DebugRenderResult();
}

void DebugRenderResult::AddLinesForBox(const AxisAlignedBoundingBox& box, const Vector3& color)
{
	RenderLine renderLine;
	renderLine.color = color;

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);

	renderLine.line.point[0] = Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z);
	renderLine.line.point[1] = Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z);
	this->AddRenderLine(renderLine);
}

//-------------------------------- RayCastResult --------------------------------

RayCastResult::RayCastResult()
{
	this->hitData.shapeID = 0;
}

/*virtual*/ RayCastResult::~RayCastResult()
{
}

/*static*/ RayCastResult* RayCastResult::Create()
{
	return new RayCastResult();
}

//-------------------------------- TransformResult --------------------------------

ObjectToWorldResult::ObjectToWorldResult()
{
}

/*virtual*/ ObjectToWorldResult::~ObjectToWorldResult()
{
}

/*static*/ ObjectToWorldResult* ObjectToWorldResult::Create()
{
	return new ObjectToWorldResult();
}

//-------------------------------- CollisionQueryResult --------------------------------

CollisionQueryResult::CollisionQueryResult()
{
	this->collisionStatusArray = new std::vector<Reference<ShapePairCollisionStatus>>();
	this->shapeID = 0;
}

/*virtual*/ CollisionQueryResult::~CollisionQueryResult()
{
	delete this->collisionStatusArray;
}

/*static*/ CollisionQueryResult* CollisionQueryResult::Create()
{
	return new CollisionQueryResult();
}

void CollisionQueryResult::AddCollisionStatus(ShapePairCollisionStatus* collisionStatus)
{
	this->collisionStatusArray->push_back(collisionStatus);
}

const ShapePairCollisionStatus* CollisionQueryResult::GetMostEgregiousCollision() const
{
	double largestLength = 0.0;
	const ShapePairCollisionStatus* foundStatus = nullptr;

	for (auto collisionStatusPair : *this->collisionStatusArray)
	{
		if (collisionStatusPair->AreInCollision())
		{
			double length = collisionStatusPair->GetSeparationDeltaLength();
			if (length > largestLength)
			{
				largestLength = length;
				foundStatus = collisionStatusPair;
			}
		}
	}

	return foundStatus;
}

Vector3 CollisionQueryResult::GetAverageSeparationDelta(ShapeID shapeID) const
{
	Vector3 averageSeparationDelta(0.0, 0.0, 0.0);
	double count = 0.0;

	for (auto collisionStatusPair : *this->collisionStatusArray)
	{
		if (collisionStatusPair->AreInCollision())
		{
			averageSeparationDelta += collisionStatusPair->GetSeparationDelta(shapeID);
			count++;
		}
	}

	return averageSeparationDelta / IMZADI_MAX(count, 1.0);
}