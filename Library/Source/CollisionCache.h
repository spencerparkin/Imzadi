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
	 * is to prevent the work of calculating a collision between two shapes from being
	 * needlessly redone, such as in the cases thus described.
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
	 * Users get read-only access to the shapes and should not try to mutate them directly.
	 * Rather, collision commands should be used to change anything about a shape.  This is because
	 * changes to a shape can cause a bunch of internal cache invalidation and spacial re-sorting
	 * to occur.  Further, such commands should not be issued until all query results have been
	 * fully processed, unless you know what you're doing, I suppose.
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
		const Shape* shapeA;			///< This is the first shape in the collision pair.  Order doesn't matter.
		const Shape* shapeB;			///< This is the second shape in the collision pair.  Again, order doesn't matter.
		uint64_t revisionNumberA;		///< This cache entry was calculated when shape A was at this revision number.
		uint64_t revisionNumberB;		///< This cache entry was calculated when shape B was at this revision number.
		Vector3 collisionCenter;		///< This is an approximate center of the overlap region between the two shapes, if they are thought to be in collision; undefined, otherwise.  It can be approximated as a contact point, I suppose.
		Vector3 separationDelta;		///< This is a minimal translation delta that, if added to shape A or subtracted from shape B, will get them into a state of at most touching.  It is undefined if the shapes are not thought to be in collision.
	};
}