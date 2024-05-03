#include "CollisionCalculator.h"
#include "CollisionCache.h"
#include "Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/Capsule.h"
#include "Math/LineSegment.h"
#include "Error.h"

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
	double radiiSum = sphereA->GetRadius() + sphereB->GetRadius();

	if (distance < radiiSum)
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = LineSegment(centerA, centerB).Lerp(sphereA->GetRadius() / radiiSum);	// TODO: Need to test this calculation.
		collisionStatus->separationDelta = centerDelta.Normalized() * (distance - radiiSum);
	}

	return collisionStatus;
}

//------------------------------ SphereCapsuleCollisionCalculator ------------------------------

SphereCapsuleCollisionCalculator::SphereCapsuleCollisionCalculator()
{
}

/*virtual*/ SphereCapsuleCollisionCalculator::~SphereCapsuleCollisionCalculator()
{
}

/*virtual*/ ShapePairCollisionStatus* SphereCapsuleCollisionCalculator::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphere = dynamic_cast<const SphereShape*>(shapeA);
	auto capsule = dynamic_cast<const CapsuleShape*>(shapeB);
	double directionFactor = -1.0;

	if (!sphere || !capsule)
	{
		sphere = dynamic_cast<const SphereShape*>(shapeB);
		capsule = dynamic_cast<const CapsuleShape*>(shapeA);
		directionFactor = 1.0;
	}

	if (!sphere || !capsule)
	{
		GetError()->AddErrorMessage("Failed to cast given shapes to sphere and capsule.");
		return nullptr;
	}

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	LineSegment capsuleSpine(capsule->GetVertex(0), capsule->GetVertex(1));
	capsuleSpine = capsule->GetObjectToWorldTransform().TransformLineSegment(capsuleSpine);
	Vector3 sphereCenter = sphere->GetObjectToWorldTransform().TransformPoint(sphere->GetCenter());

	Vector3 closestPoint = capsuleSpine.ClosestPointTo(sphereCenter);
	Vector3 delta = sphereCenter - closestPoint;
	double distance = delta.Length();
	double radiiSum = sphere->GetRadius() + capsule->GetRadius();

	if (distance < radiiSum)
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = closestPoint + delta * (capsule->GetRadius() / radiiSum);	// TODO: Need to test this calculation.
		collisionStatus->separationDelta = delta.Normalized() * (distance - radiiSum) * directionFactor;
	}

	return collisionStatus;
}

//------------------------------ CapsuleCapsuleCollisionCalculator ------------------------------

CapsuleCapsuleCollisionCalculator::CapsuleCapsuleCollisionCalculator()
{
}

/*virtual*/ CapsuleCapsuleCollisionCalculator::~CapsuleCapsuleCollisionCalculator()
{
}

/*virtual*/ ShapePairCollisionStatus* CapsuleCapsuleCollisionCalculator::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto capsuleA = static_cast<const CapsuleShape*>(shapeA);
	auto capsuleB = static_cast<const CapsuleShape*>(shapeB);

	// TODO: Use Matrix2x2 to calculate shortest line-segment connecting the spines of the given capsules.

	return nullptr;
}