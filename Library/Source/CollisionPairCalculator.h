#pragma once

#include "Defines.h"

namespace Collision
{
	class Shape;
	class CollisionPair;

	/**
	 * Derivatives of this class know how to calculate the collision information, if any, between
	 * a specific pair of collision shapes.  Such calculations constitute the narrow phase of
	 * collision detection.
	 */
	class COLLISION_LIB_API CollisionPairCalculator
	{
	public:
		CollisionPairCalculator();
		virtual ~CollisionPairCalculator();

		/**
		 * Derivatives must impliment this function to calculate the collision, if any, between the two given shapes.
		 * 
		 * @return The collision pair information should be created and returned to the caller; null, if no collision is occurring.
		 */
		virtual CollisionPair* Calculate(Shape* shapeA, Shape* shapeB) = 0;
	};

	/**
	 * This class can calculate sphere-to-sphere collisions.
	 */
	class COLLISION_LIB_API SphereSphereCollisionPairCalculator : public CollisionPairCalculator
	{
	public:
		SphereSphereCollisionPairCalculator();
		virtual ~SphereSphereCollisionPairCalculator();

		/**
		 * Perform the sphere-to-sphere collision calculations.
		 */
		virtual CollisionPair* Calculate(Shape* shapeA, Shape* shapeB) override;
	};
}