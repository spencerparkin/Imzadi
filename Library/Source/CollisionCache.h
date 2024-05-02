#pragma once

#include "Defines.h"
#include "Math/Vector3.h"
#include <unordered_map>
#include <string>

namespace Collision
{
	class Shape;
	class ShapePairCollisionStatus;
	class CollisionCalculator;

	/**
	 * Collision results are symmetric, which is to say that if you query shape A against
	 * shape B, then you will get the same result as if you had queried shape B against
	 * shgape A.  Further, if two shapes don't move and they're again queried against one
	 * another, then you will, of course, get the same result.  The purposes of this cache
	 * is to prevent the work of calculating a collision between two shapes from being redone
	 * in the cases thus described.
	 */
	class COLLISION_LIB_API CollisionCache
	{
	public:
		CollisionCache();
		virtual ~CollisionCache();

		/**
		 * This is the main entry-point into the narrow phase of collision detection.
		 * If we don't hit the cache, or the cache is invalid, a collision calculator
		 * is dispached to perform the necessary collision calculations on the given
		 * shapes.
		 * 
		 * @param[in] shapeA This is the first shape to test in a possible collision with the second shape.  Order doesn't matter.
		 * @param[in] shapeB This is the second shape to test in a possible collision with the first shape.  Again, order doesn't matter.
		 * @return The returned ShapePairCollisionState instance should be valid.  The caller does not own the memory.
		 */
		ShapePairCollisionStatus* DetermineCollisionStatusOfShapes(const Shape* shapeA, const Shape* shapeB);

		/**
		 * Wipe the cache clean, removing all pointer references to shapes.
		 */
		void Clear();

	private:

		template<typename T>
		void AddCalculator(uint32_t typeIDA, uint32_t typeIDB)
		{
			CollisionCalculator* calculator = new T();
			uint64_t calculatorKey = this->MakeCalculatorKey(typeIDA, typeIDB);
			this->calculatorMap->insert(std::pair<uint64_t, CollisionCalculator*>(calculatorKey, calculator));
		}

		void ClearCalculatorMap();

		std::string MakeCacheKey(const Shape* shapeA, const Shape* shapeB);
		uint64_t MakeCalculatorKey(const Shape* shapeA, const Shape* shapeB);
		uint64_t MakeCalculatorKey(uint32_t typeIDA, uint32_t typeIDB);

		typedef std::unordered_map<std::string, ShapePairCollisionStatus*> ShapePairCollisionStatusMap;
		ShapePairCollisionStatusMap* cacheMap;

		typedef std::unordered_map<uint64_t, CollisionCalculator*> CollisionCalculatorMap;
		CollisionCalculatorMap* calculatorMap;
	};

	/**
	 * These are the elements of the collision cache, and what are returned in a collision query.
	 * You can mutate the shapes in this pair, but note that it is not a thread-safe thing to do
	 * unless there are no command or queries pending or in flight.  Mutators on the shapes should
	 * be sure to bump revision numbers to invalidate this cache entry.
	 */
	class COLLISION_LIB_API ShapePairCollisionStatus
	{
	public:
		ShapePairCollisionStatus();
		virtual ~ShapePairCollisionStatus();

		/**
		 * Validity in this cases is not a check for Nan or Inf, but a check to see
		 * if the cache entry is still valid based on revision numbers.
		 * 
		 * @return True is returned if the cache entry is thought to still be a reflection of reality; false, otherwise.
		 */
		bool IsValid() const;

	public:
		bool inCollision;				///< Are the shapes in this pair thought to be in collision/overlapping?
		Shape* shapeA;					///< This is the first shape in the collision pair.  Order doesn't matter.
		Shape* shapeB;					///< This is the second shape in the collision pair.  Again, order doesn't matter.
		uint64_t revisionNumberA;		///< This cache entry was calculated when shape A was at this revision number.
		uint64_t revisionNumberB;		///< This cache entry was calculated when shape B was at this revision number.
		Vector3 collisionCenter;		///< This is an approximate center of the overlap region between the two shapes, if they are thought to be in collision; undefined, otherwise.  It can be approximated as a contact point, I suppose.
		Vector3 separationDelta;		///< This is a minimal translation delta that, if added to shape A or subtracted from shape B, will get them into a state of at most touching.  It is undefined if the shapes are not thought to be in collision.
	};
}