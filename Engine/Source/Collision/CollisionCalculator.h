#pragma once

#include "Defines.h"
#include "Shapes/Sphere.h"
#include "Shapes/Capsule.h"
#include "Shapes/Box.h"
#include "Shapes/Polygon.h"
#include "Math/Vector3.h"

namespace Imzadi {
namespace Collision {

class ShapePairCollisionStatus;
class Shape;

/**
 * This is the base class for all derivatives that know how to calculate the
 * collision status between a given pair of specific shape types.
 */
class IMZADI_API CollisionCalculatorInterface
{
public:
	/**
	 * Overrides should calculate and return a new collision status for the given
	 * shapes, which may or may not be in collision; that is determined by this
	 * function.
	 *
	 * Note that the order of the arguments does matter in at least two ways.  First,
	 * the override will expect certain types to be castable for each argument.
	 * Second, the separation delta is always based on moving the first shape away
	 * from the second.
	 *
	 * @param[in] shapeA The first shape to consider in a possible collision with the second.
	 * @param[in] shapeB The second shape to consider in a possible collision with the first.
	 * @return A new and valid ShapePairCollisionStatus class instance should be calculated and returned.
	 */
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) = 0;
};

/**
 * This class is just a dummy and is meant to be specialized.
 */
template<typename ShapeTypeA, typename ShapeTypeB>
class IMZADI_API CollisionCalculator : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override
	{
		IMZADI_ASSERT(false);
		return nullptr;
	}
};

/**
 * Calculate the collision status between two given sphere shapes.
 */
template<>
class IMZADI_API CollisionCalculator<SphereShape, SphereShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a sphere and a capsule.
 */
template<>
class IMZADI_API CollisionCalculator<SphereShape, CapsuleShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a capsule and a sphere.
 */
template<>
class IMZADI_API CollisionCalculator<CapsuleShape, SphereShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between two capsules.
 */
template<>
class IMZADI_API CollisionCalculator<CapsuleShape, CapsuleShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a sphere and a box.
 */
template<>
class IMZADI_API CollisionCalculator<SphereShape, BoxShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a box and a sphere.
 */
template<>
class IMZADI_API CollisionCalculator<BoxShape, SphereShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a sphere and a polygon.
 */
template<>
class IMZADI_API CollisionCalculator<SphereShape, PolygonShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a polygon and a sphere.
 */
template<>
class IMZADI_API CollisionCalculator<PolygonShape, SphereShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a pair of boxes.
 */
template<>
class IMZADI_API CollisionCalculator<BoxShape, BoxShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;

private:

	struct VertexPenetration
	{
		Vector3 surfacePoint;
		Vector3 penetrationPoint;
	};

	struct EdgeImpalement
	{
		Vector3 surfacePointA;
		Vector3 surfacePointB;
	};

	struct FacePuncture
	{
		Vector3 surfacePoint;
		Vector3 externalPoint;
		Vector3 internalPoint;
	};

	typedef std::vector<VertexPenetration> VertexPenetrationArray;
	typedef std::vector<EdgeImpalement> EdgeImpalementArray;
	typedef std::vector<FacePuncture> FacePunctureArray;

	/**
	 * Gather information about how the "away" box intersects the "home" box, if at all.
	 *
	 * @param[in] homeBox All calculations will be done in this box's space.
	 * @param[in] awayBox This box will be transformed into the space of the home box before calculations are made.
	 * @param[out] vertexPenetrationArray Away-box vertices inside the home-box are returned here in world space.
	 * @param[out] edgeImpalementArray Away-box edges originating outside the home-box and then passing in and out of it are returned here in world space.
	 * @param[out] facePunctureArray Away-box edges originating inside or outside the home-box and then entering or exiting the away-box are returned here in world space.
	 */
	bool GatherInfo(const BoxShape* homeBox, const BoxShape* awayBox, VertexPenetrationArray& vertexPenetrationArray, EdgeImpalementArray& edgeImpalementArray, FacePunctureArray& facePunctureArray);
};

/**
 * Calculate the collision status between a capsule and a polygon.
 */
template<>
class IMZADI_API CollisionCalculator<CapsuleShape, PolygonShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a polygon and a capsule.
 */
template<>
class IMZADI_API CollisionCalculator<PolygonShape, CapsuleShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

/**
 * Calculate the collision status between a box and a capsule.
 */
template<>
class IMZADI_API CollisionCalculator<BoxShape, CapsuleShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;

private:
	Vector3 CalcCapsuleDeltaToHelpExitBox(const LineSegment& capsuleSpine, double capsuleRadius, const AxisAlignedBoundingBox& axisAlignedBox);
};

/**
 * Calculate the collision status between a capsule and a box.
 */
template<>
class IMZADI_API CollisionCalculator<CapsuleShape, BoxShape> : public CollisionCalculatorInterface
{
public:
	virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
};

} // namespace Collision {
} // namespace Imzadi {