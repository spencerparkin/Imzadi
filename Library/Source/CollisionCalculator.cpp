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
#include "Math/Interval.h"
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

	BoxShape tempBoxA(*boxA);
	Vector3 totalSeparationDelta(0.0, 0.0, 0.0);

	while (true)
	{
		VertexPenetrationArray vertexPenetrationArrayA;
		EdgeImpalementArray edgeImpalementArrayA;
		FacePunctureArray facePunctureArrayA;
		bool intersectionsFoundA = this->CalculateInternal(&tempBoxA, boxB, vertexPenetrationArrayA, edgeImpalementArrayA, facePunctureArrayA);

		VertexPenetrationArray vertexPenetrationArrayB;
		EdgeImpalementArray edgeImpalementArrayB;
		FacePunctureArray facePunctureArrayB;
		bool intersectionsFoundB = this->CalculateInternal(boxB, &tempBoxA, vertexPenetrationArrayB, edgeImpalementArrayB, facePunctureArrayB);

		if (!intersectionsFoundA && !intersectionsFoundB)
			break;
		
		Vector3 separationDelta(0.0, 0.0, 0.0);

		if (edgeImpalementArrayA.size() == 1 && edgeImpalementArrayB.size() == 1 &&
			vertexPenetrationArrayA.size() == 0 && vertexPenetrationArrayB.size() == 0)
		{
			LineSegment lineA(edgeImpalementArrayA[0].surfacePointA, edgeImpalementArrayA[0].surfacePointB);
			LineSegment lineB(edgeImpalementArrayB[0].surfacePointA, edgeImpalementArrayB[0].surfacePointB);
			LineSegment connector;
			connector.SetAsShortestConnector(lineA, lineB);
			separationDelta = -connector.GetDelta();
		}
		else if (edgeImpalementArrayA.size() == 0 && edgeImpalementArrayB.size() == 0 &&
			vertexPenetrationArrayA.size() == 1 && vertexPenetrationArrayB.size() == 0)
		{
			const VertexPenetration& vertexPenetration = vertexPenetrationArrayA[0];
			LineSegment lineSeg(vertexPenetration.penetrationPoint, vertexPenetration.surfacePoint);
			separationDelta = -lineSeg.GetDelta();
		}
		else if (edgeImpalementArrayA.size() == 0 && edgeImpalementArrayB.size() == 0 &&
			vertexPenetrationArrayA.size() == 0 && vertexPenetrationArrayB.size() == 1)
		{
			const VertexPenetration& vertexPenetration = vertexPenetrationArrayB[0];
			LineSegment lineSeg(vertexPenetration.penetrationPoint, vertexPenetration.surfacePoint);
			separationDelta = lineSeg.GetDelta();
		}
		else
		{
			// This means there's a case we need to consider that we have not yet considered.
			COLL_SYS_ASSERT(false);
			break;		// Nevertheless, break out so that we can visualize what the case is that we are not yet accounting for.
		}

		Transform separationTransform;
		separationTransform.SetIdentity();
		separationTransform.translation = separationDelta;
		tempBoxA.SetObjectToWorldTransform(separationTransform * tempBoxA.GetObjectToWorldTransform());
		totalSeparationDelta += separationDelta;
	}

	if (totalSeparationDelta.IsNonZero())
	{
		collisionStatus->inCollision = true;
		collisionStatus->separationDelta = totalSeparationDelta;
	}

	return collisionStatus;
}

bool BoxBoxCollisionCalculator::CalculateInternal(const BoxShape* homeBox, const BoxShape* awayBox,
													VertexPenetrationArray& vertexPenetrationArray,
													EdgeImpalementArray& edgeImpalementArray,
													FacePunctureArray& facePunctureArray)
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
		Interval interval(0.0, edge.Length());
		Ray ray;
		ray.FromLineSegment(edge);
		ray.CastAgainst(homeBoxAligned, alphaArray);
		if (alphaArray.size() == 2)
		{
			if (interval.ContainsValue(alphaArray[0]) && interval.ContainsValue(alphaArray[1]))
			{
				EdgeImpalement edgeImpalement;
				edgeImpalement.surfacePointA = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[0]));
				edgeImpalement.surfacePointB = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[1]));
				edgeImpalementArray.push_back(edgeImpalement);
			}
			else if (interval.ContainsValue(alphaArray[0]))
			{
				FacePuncture facePuncture;
				facePuncture.surfacePoint = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[0]));
				facePuncture.externalPoint = homeToWorld.TransformPoint(edge.point[0]);
				facePuncture.internalPoint = homeToWorld.TransformPoint(edge.point[1]);
				facePunctureArray.push_back(facePuncture);
			}
		}
		else if (alphaArray.size() == 1)
		{
			if (interval.ContainsValue(alphaArray[0]))
			{
				FacePuncture facePuncture;
				facePuncture.surfacePoint = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[0]));
				facePuncture.externalPoint = homeToWorld.TransformPoint(edge.point[1]);
				facePuncture.internalPoint = homeToWorld.TransformPoint(edge.point[0]);
				facePunctureArray.push_back(facePuncture);
			}
		}
	}

	return facePunctureArray.size() > 0 || edgeSegmentArray.size() > 0 || vertexPenetrationArray.size() > 0;
}