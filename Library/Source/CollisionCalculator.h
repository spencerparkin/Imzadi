#pragma once

#include "Defines.h"
#include "Shapes/Sphere.h"
#include "Shapes/Capsule.h"
#include "Shapes/Box.h"
#include "Shapes/Polygon.h"
#include "Math/Vector3.h"

namespace Collision
{
	class ShapePairCollisionStatus;
	class Shape;

	/**
	 * This is the base class for all derivatives that know how to calculate the
	 * collision status between a given pair of specific shape types.
	 */
	class COLLISION_LIB_API CollisionCalculatorInterface
	{
	public:
		/**
		 * Overrides should calculate and return a new collision status for the given
		 * shapes, which may or may not be in collision; that is determined by this
		 * function.
		 * 
		 * @param[in] shapeA The first shape to consider in a possible collision with the second.  Order doesn't matter.
		 * @param[in] shapeB The second shape to consider in a possible collision with the first.  Again, order doesn't matter.
		 * @return A new and valid ShapePairCollisionStatus class instance should be calculated and returned.
		 */
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) = 0;
	};

	/**
	 * This class is just a dummy and is meant to be specialized.
	 */
	template<typename ShapeTypeA, typename ShapeTypeB>
	class COLLISION_LIB_API CollisionCalculator : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override
		{
			return nullptr;
		}
	};

	/**
	 * Calculate the collision status between two given sphere shapes.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<SphereShape, SphereShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a sphere and a capsule.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<SphereShape, CapsuleShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a capsule and a sphere.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<CapsuleShape, SphereShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between two capsules.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<CapsuleShape, CapsuleShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a sphere and a box.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<SphereShape, BoxShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a box and a sphere.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<BoxShape, SphereShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a sphere and a polygon.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<SphereShape, PolygonShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a polygon and a sphere.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<PolygonShape, SphereShape> : public CollisionCalculatorInterface
	{
	public:
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a pair of boxes.
	 */
	template<>
	class COLLISION_LIB_API CollisionCalculator<BoxShape, BoxShape> : public CollisionCalculatorInterface
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
}