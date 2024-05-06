#pragma once

#include "Defines.h"

namespace Collision
{
	class ShapePairCollisionStatus;
	class Shape;

	/**
	 * This is the base class for all derivatives that know how to calculate the
	 * collision status between a given pair of shapes.
	 */
	class COLLISION_LIB_API CollisionCalculator
	{
	public:
		CollisionCalculator();
		virtual ~CollisionCalculator();

		/**
		 * Overrides should calculate and return a new collision status for the given
		 * shapes, which may or may not be in collision; that is determined by this
		 * function.  Unless both shapes are the same type, the override should use a
		 * dynamic cast to determine shape is which type.
		 * 
		 * @param[in] shapeA The first shape to consider in a possible collision with the second.  Order doesn't matter.
		 * @param[in] shapeB The second shape to consider in a possible collision with the first.  Again, order doesn't matter.
		 * @return A new and valid ShapePairCollisionStatus class instance should be calculated and returned.
		 */
		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) = 0;
	};

	/**
	 * Calculate the collision status between two given sphere shapes.
	 */
	class COLLISION_LIB_API SphereSphereCollisionCalculator : public CollisionCalculator
	{
	public:
		SphereSphereCollisionCalculator();
		virtual ~SphereSphereCollisionCalculator();

		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a sphere and a capsule.
	 */
	class COLLISION_LIB_API SphereCapsuleCollisionCalculator : public CollisionCalculator
	{
	public:
		SphereCapsuleCollisionCalculator();
		virtual ~SphereCapsuleCollisionCalculator();

		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between two capsules.
	 */
	class COLLISION_LIB_API CapsuleCapsuleCollisionCalculator : public CollisionCalculator
	{
	public:
		CapsuleCapsuleCollisionCalculator();
		virtual ~CapsuleCapsuleCollisionCalculator();

		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};

	/**
	 * Calculate the collision status between a sphere and a box.
	 */
	class COLLISION_LIB_API SphereBoxCollisionCalculator : public CollisionCalculator
	{
	public:
		SphereBoxCollisionCalculator();
		virtual ~SphereBoxCollisionCalculator();

		virtual ShapePairCollisionStatus* Calculate(const Shape* shapeA, const Shape* shapeB) override;
	};
}