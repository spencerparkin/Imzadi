#include "CollisionCalculator.h"
#include "CollisionCache.h"
#include "Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/Capsule.h"
#include "Shapes/Box.h"
#include "Shapes/Polygon.h"
#include "Math/LineSegment.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
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
	auto sphereA = dynamic_cast<const SphereShape*>(shapeA);
	auto sphereB = dynamic_cast<const SphereShape*>(shapeB);

	if (!sphereA || !sphereB)
	{
		GetError()->AddErrorMessage("Failed to cast given shapes to spheres.");
		return nullptr;
	}

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
	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	auto capsuleA = dynamic_cast<const CapsuleShape*>(shapeA);
	auto capsuleB = dynamic_cast<const CapsuleShape*>(shapeB);

	if (!capsuleA || !capsuleB)
	{
		GetError()->AddErrorMessage("Failed to cast given shapes to capsules.");
		return nullptr;
	}

	LineSegment spineA = capsuleA->GetObjectToWorldTransform().TransformLineSegment(capsuleA->GetSpine());
	LineSegment spineB = capsuleB->GetObjectToWorldTransform().TransformLineSegment(capsuleB->GetSpine());

	LineSegment shortestConnector;
	if (shortestConnector.SetAsShortestConnector(spineA, spineB))
	{
		double distance = shortestConnector.Length();
		double radiiSum = capsuleA->GetRadius() + capsuleB->GetRadius();

		if (distance < radiiSum)
		{
			collisionStatus->inCollision = true;
			collisionStatus->collisionCenter = Vector3(0.0, 0.0, 0.0);	// TODO: Figure this out.
			collisionStatus->separationDelta = shortestConnector.GetDelta().Normalized() * (distance - radiiSum);
		}
	}

	return collisionStatus;
}

//------------------------------ SphereBoxCollisionCalculator ------------------------------

SphereBoxCollisionCalculator::SphereBoxCollisionCalculator()
{
}

/*virtual*/ SphereBoxCollisionCalculator::~SphereBoxCollisionCalculator()
{
}

/*virtual*/ ShapePairCollisionStatus* SphereBoxCollisionCalculator::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphere = dynamic_cast<const SphereShape*>(shapeA);
	auto box = dynamic_cast<const BoxShape*>(shapeB);
	double directionFactor = 1.0;

	if (!sphere || !box)
	{
		sphere = dynamic_cast<const SphereShape*>(shapeB);
		box = dynamic_cast<const BoxShape*>(shapeA);
		directionFactor = -1.0;
	}

	if (!sphere || !box)
	{
		GetError()->AddErrorMessage("Failed to cast given shapes to sphere and box.");
		return nullptr;
	}

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	Transform worldToBox = box->GetWorldToObjectTransform();
	Transform sphereToWorld = sphere->GetObjectToWorldTransform();
	Vector3 sphereCenter = worldToBox.TransformPoint(sphereToWorld.TransformPoint(sphere->GetCenter()));

	AxisAlignedBoundingBox objectSpaceBox;
	box->GetAxisAlignedBox(objectSpaceBox);

	Vector3 closestBoxPoint = objectSpaceBox.ClosestPointTo(sphereCenter);

	Vector3 delta = sphereCenter - closestBoxPoint;
	double distance = delta.Length();
	if (distance < sphere->GetRadius())
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = Vector3(0.0, 0.0, 0.0);	// TODO: Figure this out.

		double boxBorderThickness = 1e-4;
		if (distance < boxBorderThickness)
		{
			collisionStatus->separationDelta = closestBoxPoint.Normalized() * sphere->GetRadius() * directionFactor;
		}
		else if (objectSpaceBox.ContainsPoint(sphereCenter))
		{
			collisionStatus->separationDelta = -delta.Normalized() * (sphere->GetRadius() + distance) * directionFactor;
		}
		else
		{
			collisionStatus->separationDelta = delta.Normalized() * (sphere->GetRadius() - distance) * directionFactor;
		}

		collisionStatus->separationDelta = box->GetObjectToWorldTransform().TransformNormal(collisionStatus->separationDelta);
	}

	return collisionStatus;
}

//------------------------------ SpherePolygonCollisionCalculator ------------------------------

SpherePolygonCollisionCalculator::SpherePolygonCollisionCalculator()
{
}

/*virtual*/ SpherePolygonCollisionCalculator::~SpherePolygonCollisionCalculator()
{
}

/*virtual*/ ShapePairCollisionStatus* SpherePolygonCollisionCalculator::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphere = dynamic_cast<const SphereShape*>(shapeA);
	auto polygon = dynamic_cast<const PolygonShape*>(shapeB);
	double directionFactor = 1.0;

	if (!sphere || !polygon)
	{
		sphere = dynamic_cast<const SphereShape*>(shapeB);
		polygon = dynamic_cast<const PolygonShape*>(shapeA);
		directionFactor = -1.0;
	}

	if (!sphere || !polygon)
	{
		GetError()->AddErrorMessage("Failed to cast given shapes to sphere and polygon.");
		return nullptr;
	}

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	Vector3 sphereCenter = sphere->GetObjectToWorldTransform().TransformPoint(sphere->GetCenter());
	Vector3 polygonPoint = polygon->ClosestPointTo(sphereCenter);
	Vector3 delta = sphereCenter - polygonPoint;
	double distance = delta.Length();
	if (distance < sphere->GetRadius())
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = polygonPoint;
		if(!delta.Normalize())
			delta = polygon->GetPlane().unitNormal;
			
		collisionStatus->separationDelta = delta * (sphere->GetRadius() - distance) * directionFactor;
	}

	return collisionStatus;
}

//------------------------------ BoxBoxCollisionCalculator ------------------------------

BoxBoxCollisionCalculator::BoxBoxCollisionCalculator()
{
}

/*virtual*/ BoxBoxCollisionCalculator::~BoxBoxCollisionCalculator()
{
}

/*virtual*/ ShapePairCollisionStatus* BoxBoxCollisionCalculator::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto boxA = dynamic_cast<const BoxShape*>(shapeA);
	auto boxB = dynamic_cast<const BoxShape*>(shapeB);

	if (!boxA || !boxB)
	{
		GetError()->AddErrorMessage("Failed to cast given shapes to box shapes.");
		return nullptr;
	}

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	VertexPenetrationArray vertexPenetrationArrayA;
	EdgeImpalementArray edgeImpalementArrayA;
	this->CalculateInternal(boxA, boxB, vertexPenetrationArrayA, edgeImpalementArrayA);

	VertexPenetrationArray vertexPenetrationArrayB;
	EdgeImpalementArray edgeImpalementArrayB;
	this->CalculateInternal(boxB, boxA, vertexPenetrationArrayB, edgeImpalementArrayB);

	if (vertexPenetrationArrayA.size() > 0 || edgeImpalementArrayA.size() > 0 ||
		vertexPenetrationArrayB.size() > 0 || edgeImpalementArrayB.size() > 0)
	{
		collisionStatus->inCollision = true;

		if (edgeImpalementArrayA.size() == 1 && edgeImpalementArrayB.size() == 1)
		{
			LineSegment lineA(edgeImpalementArrayA[0].surfacePointA, edgeImpalementArrayA[0].surfacePointB);
			LineSegment lineB(edgeImpalementArrayB[0].surfacePointA, edgeImpalementArrayB[0].surfacePointB);
			LineSegment connector;
			connector.SetAsShortestConnector(lineA, lineB);
			collisionStatus->collisionCenter = connector.Lerp(0.5);
			collisionStatus->separationDelta = connector.GetDelta();
		}
		//else...TODO: Handle other cases here.
	}

	return collisionStatus;
}

void BoxBoxCollisionCalculator::CalculateInternal(const BoxShape* homeBox, const BoxShape* awayBox, VertexPenetrationArray& vertexPenetrationArray, EdgeImpalementArray& edgeImpalementArray)
{
	AxisAlignedBoundingBox homeBoxAligned;
	homeBox->GetAxisAlignedBox(homeBoxAligned);

	std::vector<Vector3> awayCornerPointArray;
	awayBox->GetCornerPointArray(awayCornerPointArray, false);

	const Transform& homeToWorld = homeBox->GetObjectToWorldTransform();
	Transform worldToHome = homeToWorld.Inverted();

	const Transform& awayToWorld = awayBox->GetObjectToWorldTransform();
	Transform awayToHome = worldToHome * awayToWorld;

	for (Vector3& awayCorner : awayCornerPointArray)
		awayCorner = awayToHome.TransformPoint(awayCorner);

	for (const Vector3& awayCorner : awayCornerPointArray)
	{
		if (homeBoxAligned.ContainsPoint(awayCorner))
		{
			VertexPenetration vertexPenetration;
			vertexPenetration.penetrationPoint = homeToWorld.TransformPoint(awayCorner);
			vertexPenetration.surfacePoint = homeToWorld.TransformPoint(homeBoxAligned.ClosestPointTo(awayCorner));
			vertexPenetrationArray.push_back(vertexPenetration);
		}
	}

	std::vector<LineSegment> edgeSegmentArray;
	awayBox->GetEdgeSegmentArray(edgeSegmentArray, false);

	for (LineSegment& edge : edgeSegmentArray)
		edge = awayToHome.TransformLineSegment(edge);

	for (const LineSegment& edge : edgeSegmentArray)
	{
		std::vector<double> alphaArray;
		Ray ray;
		ray.FromLineSegment(edge);
		ray.CastAgainst(homeBoxAligned, alphaArray);
		if (alphaArray.size() == 2)
		{
			EdgeImpalement edgeImpalement;
			edgeImpalement.surfacePointA = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[0]));
			edgeImpalement.surfacePointB = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[1]));
			edgeImpalementArray.push_back(edgeImpalement);
		}
	}
}