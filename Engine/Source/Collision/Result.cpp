#include "Result.h"
#include "CollisionCache.h"
#include "Math/AxisAlignedBoundingBox.h"

using namespace Imzadi;
using namespace Imzadi::Collision;

//-------------------------------- Result --------------------------------

Result::Result()
{
}

/*virtual*/ Result::~Result()
{
}

//-------------------------------- BoolResult --------------------------------

BoolResult::BoolResult()
{
	this->answer = false;
}

/*virtual*/ BoolResult::~BoolResult()
{
}

//-------------------------------- StringResult --------------------------------

StringResult::StringResult()
{
	this->text = "";
}

/*virtual*/ StringResult::~StringResult()
{
}

//-------------------------------- StatsResult --------------------------------

StatsResult::StatsResult()
{
	this->numShapes = 0;
}

/*virtual*/ StatsResult::~StatsResult()
{
}

//-------------------------------- DebugRenderResult --------------------------------

DebugRenderResult::DebugRenderResult()
{
}

/*virtual*/ DebugRenderResult::~DebugRenderResult()
{
}

void DebugRenderResult::AddRenderLine(const RenderLine& renderLine)
{
	this->renderLineArray.push_back(renderLine);
}

void DebugRenderResult::AddLinesForBox(const AxisAlignedBoundingBox& box, const Vector3& color)
{
	RenderLine renderLine;
	renderLine.color = color;

	std::vector<LineSegment> edgeSegmentArray;
	box.GetEdgeSegments(edgeSegmentArray);

	for (const auto& segment : edgeSegmentArray)
	{
		renderLine.line = segment;
		this->AddRenderLine(renderLine);
	}
}

//-------------------------------- RayCastResult --------------------------------

RayCastResult::RayCastResult()
{
	this->hitData.shapeID = 0;
	this->hitData.shape = nullptr;
	this->hitData.alpha = 0.0;
}

/*virtual*/ RayCastResult::~RayCastResult()
{
}

//-------------------------------- TransformResult --------------------------------

ObjectToWorldResult::ObjectToWorldResult()
{
}

/*virtual*/ ObjectToWorldResult::~ObjectToWorldResult()
{
}

//-------------------------------- CollisionQueryResult --------------------------------

CollisionQueryResult::CollisionQueryResult()
{
	this->shapeID = 0;
}

/*virtual*/ CollisionQueryResult::~CollisionQueryResult()
{
}

void CollisionQueryResult::AddCollisionStatus(ShapePairCollisionStatus* collisionStatus)
{
	this->collisionStatusArray.push_back(collisionStatus);
}

const ShapePairCollisionStatus* CollisionQueryResult::GetMostEgregiousCollision() const
{
	double largestLength = 0.0;
	const ShapePairCollisionStatus* foundStatus = nullptr;

	for (auto collisionStatusPair : this->collisionStatusArray)
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

	for (auto collisionStatusPair : this->collisionStatusArray)
	{
		if (collisionStatusPair->AreInCollision())
		{
			averageSeparationDelta += collisionStatusPair->GetSeparationDelta(shapeID);
			count++;
		}
	}

	return averageSeparationDelta / IMZADI_MAX(count, 1.0);
}