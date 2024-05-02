#include "CollisionCalculator.h"
#include "CollisionCache.h"
#include "Shape.h"
#include "Shapes/Sphere.h"
#include "Math/LineSegment.h"

using namespace Collision;

//------------------------------ CollisionCalculator ------------------------------

CollisionCalculator::CollisionCalculator()
{
}

/*virtual*/ CollisionCalculator::~CollisionCalculator()
{
}

//------------------------------ SphereSphereCollisionCalculator ------------------------------

SphereSphereCollisionCalculator::SphereSphereCollisionCalculator()
{
}

/*virtual*/ SphereSphereCollisionCalculator::~SphereSphereCollisionCalculator()
{
}

/*virtual*/ ShapePairCollisionStatus* SphereSphereCollisionCalculator::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphereA = static_cast<const SphereShape*>(shapeA);
	auto sphereB = static_cast<const SphereShape*>(shapeB);

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	Vector3 centerA = sphereA->GetObjectToWorldTransform().TransformPoint(sphereA->GetCenter());
	Vector3 centerB = sphereB->GetObjectToWorldTransform().TransformPoint(sphereB->GetCenter());

	Vector3 centerDelta = centerB - centerA;
	double distance = centerDelta.Length();
	double radiiSum = sphereA->GetRadias() + sphereB->GetRadias();

	if (distance < radiiSum)
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = LineSegment(centerA, centerB).Lerp(sphereA->GetRadias() / radiiSum);
		collisionStatus->separationDelta = centerDelta.Normalized() * (distance - radiiSum);
	}

	return collisionStatus;
}