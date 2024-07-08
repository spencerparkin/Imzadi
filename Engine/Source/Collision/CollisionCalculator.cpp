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
#include <algorithm>

using namespace Imzadi;

//------------------------------ CollisionCalculator<SphereShape, SphereShape> ------------------------------

ShapePairCollisionStatus* CollisionCalculator<SphereShape, SphereShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphereA = dynamic_cast<const SphereShape*>(shapeA);
	auto sphereB = dynamic_cast<const SphereShape*>(shapeB);

	if (!sphereA || !sphereB)
		return nullptr;

	auto collisionStatus = new ShapePairCollisionStatus(sphereA, sphereB);

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

//------------------------------ CollisionCalculator<SphereShape, CapsuleShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<SphereShape, CapsuleShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphere = dynamic_cast<const SphereShape*>(shapeA);
	auto capsule = dynamic_cast<const CapsuleShape*>(shapeB);
	
	if (!sphere || !capsule)
		return nullptr;

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
		collisionStatus->separationDelta = delta.Normalized() * (radiiSum - distance);
	}

	return collisionStatus;
}

//------------------------------ CollisionCalculator<CapsuleShape, SphereShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<CapsuleShape, SphereShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	ShapePairCollisionStatus* collisionStatus = CollisionCalculator<SphereShape, CapsuleShape>().Calculate(shapeB, shapeA);
	if (collisionStatus)
		collisionStatus->FlipContext();
	return collisionStatus;
}

//------------------------------ CollisionCalculator<CapsuleShape, CapsuleShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<CapsuleShape, CapsuleShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	auto capsuleA = dynamic_cast<const CapsuleShape*>(shapeA);
	auto capsuleB = dynamic_cast<const CapsuleShape*>(shapeB);

	if (!capsuleA || !capsuleB)
		return nullptr;

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

//------------------------------ CollisionCalculator<SphereShape, BoxShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<SphereShape, BoxShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphere = dynamic_cast<const SphereShape*>(shapeA);
	auto box = dynamic_cast<const BoxShape*>(shapeB);

	if (!sphere || !box)
		return nullptr;

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	Transform worldToBox = box->GetWorldToObjectTransform();
	Transform sphereToWorld = sphere->GetObjectToWorldTransform();
	Vector3 sphereCenter = worldToBox.TransformPoint(sphereToWorld.TransformPoint(sphere->GetCenter()));

	AxisAlignedBoundingBox objectSpaceBox;
	box->GetAxisAlignedBox(objectSpaceBox);

	Vector3 closestBoxPoint = objectSpaceBox.ClosestPointTo(sphereCenter);

	Vector3 delta = sphereCenter - closestBoxPoint;
	double distance = delta.Length();
	if (distance < sphere->GetRadius() || box->ContainsPoint(sphereCenter))
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = Vector3(0.0, 0.0, 0.0);	// TODO: Figure this out.

		double boxBorderThickness = 1e-4;
		if (distance < boxBorderThickness)
		{
			Vector3 unitDirection = (closestBoxPoint - objectSpaceBox.GetCenter()).Normalized();
			double angleX = unitDirection.AngleBetween(Vector3(1.0, 0.0, 0.0));
			double angleY = unitDirection.AngleBetween(Vector3(0.0, 1.0, 0.0));
			double angleZ = unitDirection.AngleBetween(Vector3(0.0, 0.0, 1.0));

			if (angleX < angleY && angleX < angleZ)
				collisionStatus->separationDelta = Vector3(sphere->GetRadius(), 0.0, 0.0);
			else if(angleY < angleX && angleY < angleZ)
				collisionStatus->separationDelta = Vector3(0.0, sphere->GetRadius(), 0.0);
			else
				collisionStatus->separationDelta = Vector3(0.0, 0.0, sphere->GetRadius());
		}
		else if (objectSpaceBox.ContainsPoint(sphereCenter))
		{
			collisionStatus->separationDelta = -delta.Normalized() * (sphere->GetRadius() + distance);
		}
		else
		{
			collisionStatus->separationDelta = delta.Normalized() * (sphere->GetRadius() - distance);
		}

		collisionStatus->separationDelta = box->GetObjectToWorldTransform().TransformVector(collisionStatus->separationDelta);
	}

	return collisionStatus;
}

//------------------------------ CollisionCalculator<BoxShape, SphereShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<BoxShape, SphereShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	ShapePairCollisionStatus* collisionStatus = CollisionCalculator<SphereShape, BoxShape>().Calculate(shapeB, shapeA);
	if (collisionStatus)
		collisionStatus->FlipContext();
	return collisionStatus;
}

//------------------------------ CollisionCalculator<SphereShape, PolygonShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<SphereShape, PolygonShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto sphere = dynamic_cast<const SphereShape*>(shapeA);
	auto polygon = dynamic_cast<const PolygonShape*>(shapeB);

	if (!sphere || !polygon)
		return nullptr;

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
			
		collisionStatus->separationDelta = delta * (sphere->GetRadius() - distance);
	}

	return collisionStatus;
}

//------------------------------ CollisionCalculator<PolygonShape, SphereShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<PolygonShape, SphereShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	ShapePairCollisionStatus* collisionStatus = CollisionCalculator<SphereShape, PolygonShape>().Calculate(shapeB, shapeA);
	if (collisionStatus)
		collisionStatus->FlipContext();
	return collisionStatus;
}

//------------------------------ CollisionCalculator<BoxShape, BoxShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<BoxShape, BoxShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto boxA = dynamic_cast<const BoxShape*>(shapeA);
	auto boxB = dynamic_cast<const BoxShape*>(shapeB);

	if (!boxA || !boxB)
		return nullptr;

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	BoxShape tempBoxA(*boxA);
	Vector3 totalSeparationDelta(0.0, 0.0, 0.0);

	while (true)
	{
		VertexPenetrationArray vertexPenetrationArrayA;
		EdgeImpalementArray edgeImpalementArrayA;
		FacePunctureArray facePunctureArrayA;
		bool intersectionsFoundA = this->GatherInfo(&tempBoxA, boxB, vertexPenetrationArrayA, edgeImpalementArrayA, facePunctureArrayA);

		VertexPenetrationArray vertexPenetrationArrayB;
		EdgeImpalementArray edgeImpalementArrayB;
		FacePunctureArray facePunctureArrayB;
		bool intersectionsFoundB = this->GatherInfo(boxB, &tempBoxA, vertexPenetrationArrayB, edgeImpalementArrayB, facePunctureArrayB);

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
		else if(facePunctureArrayA.size() > 0 || facePunctureArrayB.size() > 0)
		{
			std::vector<const FacePuncture*> combinedPunctureArray;
			for (const FacePuncture& facePuncture : facePunctureArrayA)
				combinedPunctureArray.push_back(&facePuncture);
			for (const FacePuncture& facePuncture : facePunctureArrayB)
				combinedPunctureArray.push_back(&facePuncture);
			double smallestPunctureDistance = std::numeric_limits<double>::max();
			int j = -1;
			for (int i = 0; i < (signed)combinedPunctureArray.size(); i++)
			{
				const FacePuncture* facePuncture = combinedPunctureArray[i];
				double punctureDistance = (facePuncture->surfacePoint - facePuncture->internalPoint).Length();
				if (punctureDistance < smallestPunctureDistance)
				{
					smallestPunctureDistance = punctureDistance;
					j = i;
				}
			}
			const FacePuncture* chosenPuncture = combinedPunctureArray[j];
			separationDelta = chosenPuncture->internalPoint - chosenPuncture->surfacePoint;
			if (j >= facePunctureArrayA.size())
				separationDelta = -separationDelta;
		}
		else
		{
			// This means there's a case we need to consider that we have not yet considered.
			IMZADI_ASSERT(false);
			break;
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

bool CollisionCalculator<BoxShape, BoxShape>::GatherInfo(const BoxShape* homeBox, const BoxShape* awayBox,
															VertexPenetrationArray& vertexPenetrationArray,
															EdgeImpalementArray& edgeImpalementArray,
															FacePunctureArray& facePunctureArray)
{
	constexpr double threshold = 1e-5;

	vertexPenetrationArray.clear();
	edgeImpalementArray.clear();
	facePunctureArray.clear();

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
			if ((vertexPenetration.surfacePoint - vertexPenetration.penetrationPoint).Length() > threshold)
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
			constexpr double epsilon = 1e-6;
			if (interval.ContainsValue(alphaArray[0], epsilon) && interval.ContainsValue(alphaArray[1], epsilon))
			{
				EdgeImpalement edgeImpalement;
				edgeImpalement.surfacePointA = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[0]));
				edgeImpalement.surfacePointB = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[1]));
				if ((edgeImpalement.surfacePointA - edgeImpalement.surfacePointB).Length() > threshold)
					edgeImpalementArray.push_back(edgeImpalement);
			}
			else if (interval.ContainsValue(alphaArray[0]))
			{
				FacePuncture facePuncture;
				facePuncture.surfacePoint = homeToWorld.TransformPoint(ray.CalculatePoint(alphaArray[0]));
				facePuncture.externalPoint = homeToWorld.TransformPoint(edge.point[0]);
				facePuncture.internalPoint = homeToWorld.TransformPoint(edge.point[1]);
				if ((facePuncture.surfacePoint - facePuncture.internalPoint).Length() > threshold)
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
				if ((facePuncture.surfacePoint - facePuncture.internalPoint).Length() > threshold)
					facePunctureArray.push_back(facePuncture);
			}
		}
	}

	return facePunctureArray.size() > 0 || edgeImpalementArray.size() > 0 || vertexPenetrationArray.size() > 0;
}

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<CapsuleShape, PolygonShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto capsule = dynamic_cast<const CapsuleShape*>(shapeA);
	auto polygon = dynamic_cast<const PolygonShape*>(shapeB);

	if (!capsule || !polygon)
		return nullptr;

	LineSegment capsuleSpine = capsule->GetObjectToWorldTransform().TransformLineSegment(capsule->GetSpine());
	
	Vector3 intersectionPoint;
	bool intersectsSpine = polygon->IntersectsWith(capsuleSpine, intersectionPoint);

	const Plane& worldPlane = polygon->GetWorldPlane();

	std::vector<LineSegment> connectorArray;

	Vector3 closestPoint0 = worldPlane.ClosestPointTo(capsuleSpine.point[0]);
	if (polygon->ContainsPoint(closestPoint0))
		connectorArray.push_back(LineSegment(closestPoint0, capsuleSpine.point[0]));

	Vector3 closestPoint1 = worldPlane.ClosestPointTo(capsuleSpine.point[1]);
	if (polygon->ContainsPoint(closestPoint1))
		connectorArray.push_back(LineSegment(closestPoint1, capsuleSpine.point[1]));

	std::vector<LineSegment> edgeArray;
	polygon->GetWorldEdges(edgeArray);
	for (const LineSegment& edge : edgeArray)
	{
		LineSegment connector;
		if (connector.SetAsShortestConnector(edge, capsuleSpine))
			connectorArray.push_back(connector);
	}

	const LineSegment* shortestConnector = nullptr;
	double shortestDistance = std::numeric_limits<double>::max();
	for (const LineSegment& connector : connectorArray)
	{
		double distance = connector.Length();
		if (distance < shortestDistance)
		{
			shortestDistance = distance;
			shortestConnector = &connector;
		}
	}

	IMZADI_ASSERT(shortestConnector != nullptr);
	
	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	if (intersectsSpine || shortestDistance < capsule->GetRadius())
	{
		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = shortestConnector->point[0];	// TODO: This is dubious.  Fix it.

		Vector3 delta = shortestConnector->GetDelta();
		double distance = 0.0;
		if (!delta.Normalize(&distance))
		{
			// TODO: Handle this case.
			IMZADI_ASSERT(false);
		}
		else
		{
			if (intersectsSpine)
				collisionStatus->separationDelta = -delta * (capsule->GetRadius() + distance);
			else
				collisionStatus->separationDelta = delta * (capsule->GetRadius() - shortestDistance);
		}
	}

	return collisionStatus;
}

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<PolygonShape, CapsuleShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	ShapePairCollisionStatus* collisionStatus = CollisionCalculator<CapsuleShape, PolygonShape>().Calculate(shapeB, shapeA);
	if (collisionStatus)
		collisionStatus->FlipContext();
	return collisionStatus;
}

//------------------------------ CollisionCalculator<BoxShape, CapsuleShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<BoxShape, CapsuleShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	auto box = dynamic_cast<const BoxShape*>(shapeA);
	auto capsule = dynamic_cast<const CapsuleShape*>(shapeB);

	if (!box || !capsule)
		return nullptr;

	auto collisionStatus = new ShapePairCollisionStatus(shapeA, shapeB);

	Transform worldToBox = box->GetWorldToObjectTransform();
	Transform capsuleToWorld = capsule->GetObjectToWorldTransform();
	Transform capsuleToBox = worldToBox * capsuleToWorld;

	LineSegment capsuleSpine = capsuleToBox.TransformLineSegment(capsule->GetSpine());
	LineSegment movingCapsuleSpine = capsuleSpine;

	AxisAlignedBoundingBox axisAlignedBox;
	box->GetAxisAlignedBox(axisAlignedBox);

	while (true)
	{
		Vector3 delta = this->CalcCapsuleDeltaToHelpExitBox(movingCapsuleSpine, capsule->GetRadius(), axisAlignedBox);
		if (delta.Dot(delta) == 0.0)
			break;

		movingCapsuleSpine.point[0] += delta;
		movingCapsuleSpine.point[1] += delta;
	}

	collisionStatus->separationDelta = capsuleSpine.point[0] - movingCapsuleSpine.point[0];
	if (collisionStatus->separationDelta.Length() > 0.0)
	{
		Transform boxToWorld = box->GetObjectToWorldTransform();
		collisionStatus->separationDelta = boxToWorld.TransformVector(collisionStatus->separationDelta);

		collisionStatus->inCollision = true;
		collisionStatus->collisionCenter = Vector3(0.0, 0.0, 0.0);	// TODO: Punt on this for now.
	}

	return collisionStatus;
}

Vector3 CollisionCalculator<BoxShape, CapsuleShape>::CalcCapsuleDeltaToHelpExitBox(const LineSegment& capsuleSpine, double capsuleRadius, const AxisAlignedBoundingBox& axisAlignedBox)
{
	static double borderThickness = 1e-4;
	static double epsilon = 1e-6;

	//
	// Case #1: Both vertices of the spine are interior to the box.
	//

	if (axisAlignedBox.ContainsInteriorPoint(capsuleSpine.point[0], borderThickness) &&
		axisAlignedBox.ContainsInteriorPoint(capsuleSpine.point[1], borderThickness))
	{
		Vector3 pointA = axisAlignedBox.ClosestPointTo(capsuleSpine.point[0]);
		Vector3 pointB = axisAlignedBox.ClosestPointTo(capsuleSpine.point[1]);

		Vector3 deltaA = pointA - capsuleSpine.point[0];
		Vector3 deltaB = pointB - capsuleSpine.point[1];

		double distanceA = deltaA.Length();
		double distanceB = deltaB.Length();

		if (distanceA < distanceB)
			return deltaA;
		else
			return deltaB;
	}

	//
	// Case #2: Exactly one of the two vertices is interior to the box.
	//

	for (int i = 0; i < 2; i++)
	{
		if (axisAlignedBox.ContainsInteriorPoint(capsuleSpine.point[i]))
		{
			Vector3 point = axisAlignedBox.ClosestPointTo(capsuleSpine.point[i]);
			Vector3 delta = point - capsuleSpine.point[i];
			return delta;
		}
	}

	//
	// Case #3: Both vertices are outside or on the border of the box.
	//

	struct EdgeData
	{
		LineSegment connector;
		const LineSegment* edge;
	};

	std::vector<EdgeData> edgeDataArray;
	std::vector<LineSegment> edgeSegmentArray;
	axisAlignedBox.GetEdgeSegments(edgeSegmentArray);
	for (const auto& edgeSegment : edgeSegmentArray)
	{
		LineSegment connector;
		if (connector.SetAsShortestConnector(capsuleSpine, edgeSegment))
			edgeDataArray.push_back({ connector, &edgeSegment });
	}

	std::sort(edgeDataArray.begin(), edgeDataArray.end(), [](const EdgeData& edgeDataA, const EdgeData& edgeDataB) -> bool {
		return edgeDataA.connector.SquareLength() < edgeDataB.connector.SquareLength();
	});

	//
	// Case #3A: The vertices are on or beyond the same face plane of the box.
	//

	std::vector<Plane> sidePlaneArray;
	axisAlignedBox.GetSidePlanes(sidePlaneArray);
	for (const auto& sidePlane : sidePlaneArray)
	{
		Plane::Side sideA = sidePlane.GetSide(capsuleSpine.point[0], borderThickness);
		Plane::Side sideB = sidePlane.GetSide(capsuleSpine.point[1], borderThickness);
		if (sideA != Plane::Side::BACK && sideB != Plane::Side::BACK)
		{
			const LineSegment* shortestConnector = (edgeDataArray.size() > 0) ? &edgeDataArray[0].connector : nullptr;
			if (!shortestConnector || fabs(shortestConnector->Length() - capsuleRadius) > epsilon)
			{
				double distanceA = sidePlane.SignedDistanceTo(capsuleSpine.point[0]);
				double distanceB = sidePlane.SignedDistanceTo(capsuleSpine.point[1]);

				Vector3 delta;

				if (distanceA < distanceB)
				{
					if (distanceA < capsuleRadius)
						delta = sidePlane.unitNormal * (capsuleRadius - distanceA);
				}
				else
				{
					if (distanceB < capsuleRadius)
						delta = sidePlane.unitNormal * (capsuleRadius - distanceB);
				}

				return delta;
			}
		}
	}

	//
	// Case #3B: The vertices are on or beyond different face planes of the box.
	//

	for (const auto& edgeData : edgeDataArray)
	{
		const LineSegment& connector = edgeData.connector;
		const Vector3& capsuleSpinePoint = connector.point[0];
		if (axisAlignedBox.ContainsInteriorPoint(capsuleSpinePoint))
		{
			Vector3 vector = connector.GetDelta();
			double distance = vector.Length();
			Vector3 delta = vector * (capsuleRadius + distance) / distance;
			return delta;
		}
		else if (axisAlignedBox.ContainsPointOnSurface(capsuleSpinePoint))
		{
			Vector3 vector = edgeData.edge->GetDelta().Cross(capsuleSpine.GetDelta());
			double distanceA = (capsuleSpinePoint + vector - axisAlignedBox.GetCenter()).Length();
			double distanceB = (capsuleSpinePoint - vector - axisAlignedBox.GetCenter()).Length();
			if (distanceB > distanceA)
				vector *= -1.0;
			Vector3 delta = vector.Normalized() * capsuleRadius;
			return delta;
		}
		else
		{
			Vector3 vector = connector.GetDelta();
			double distance = vector.Length();
			if (distance < capsuleRadius)
			{
				Vector3 delta = vector * (capsuleRadius - distance) / distance;
				double length = delta.Length();
				if (length < epsilon)
					delta.SetComponents(0.0, 0.0, 0.0);
				return delta;
			}
		}
	}

	return Vector3(0.0, 0.0, 0.0);
}

//------------------------------ CollisionCalculator<CapsuleShape, BoxShape> ------------------------------

/*virtual*/ ShapePairCollisionStatus* CollisionCalculator<CapsuleShape, BoxShape>::Calculate(const Shape* shapeA, const Shape* shapeB)
{
	ShapePairCollisionStatus* collisionStatus = CollisionCalculator<BoxShape, CapsuleShape>().Calculate(shapeB, shapeA);
	if (collisionStatus)
		collisionStatus->FlipContext();
	return collisionStatus;
}