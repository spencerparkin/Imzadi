#pragma once

#include "Defines.h"
#include "Math/Vector3.h"
#include "CollisionCalculator.h"
#include "Shape.h"
#include <unordered_map>
#include <string>

namespace Collision
{
	class ShapePairCollisionStatus;

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

		template<typename ShapeTypeA, typename ShapeTypeB>
		void AddCalculator()
		{
			CollisionCalculatorInterface* calculator = new CollisionCalculator<ShapeTypeA, ShapeTypeB>();
			uint32_t typeIDA = ShapeTypeA::StaticTypeID();
			uint32_t typeIDB = ShapeTypeB::StaticTypeID();
			uint64_t calculatorKey = this->MakeCalculatorKey(typeIDA, typeIDB);
			this->calculatorMap->insert(std::pair<uint64_t, CollisionCalculatorInterface*>(calculatorKey, calculator));
		}

		void ClearCalculatorMap();

		std::string MakeCacheKey(const Shape* shapeA, const Shape* shapeB);
		uint64_t MakeCalculatorKey(const Shape* shapeA, const Shape* shapeB);
		uint64_t MakeCalculatorKey(uint32_t typeIDA, uint32_t typeIDB);

		typedef std::unordered_map<std::string, ShapePairCollisionStatus*> ShapePairCollisionStatusMap;
		ShapePairCollisionStatusMap* cacheMap;

		typedef std::unordered_map<uint64_t, CollisionCalculatorInterface*> CollisionCalculatorMap;
		CollisionCalculatorMap* calculatorMap;
	};

	/**
	 * These are the elements of the collision cache, and what are returned in a collision query result.
	 * Note that raw shape pointers are included in this class/structure, but should not be accessed
	 * by the collision system user.  They are made private, but don't be tempted to hack the structure,
	 * because read/write or even just read-only access to them is not thread-safe.
	 */
	class COLLISION_LIB_API ShapePairCollisionStatus
	{
	public:
		ShapePairCollisionStatus(const Shape* shapeA, const Shape* shapeB);
		virtual ~ShapePairCollisionStatus();

		/**
		 * Validity in this cases is not a check for Nan or Inf, but a check to see
		 * if the cache entry is still valid based on revision numbers.
		 * 
		 * @return True is returned if the cache entry is thought to still be a reflection of reality; false, otherwise.
		 */
		bool IsValid() const;

		/**
		 * Get the ID of one of the two shapes involved in this collision status pair.
		 * 
		 * @param[in] i If this is even, shape A's ID is returned; B, otherwise.
		 */
		ShapeID GetShapeID(int i) const;

		/**
		 * Return the minimal translation vector that would move the shape with the given ID in
		 * such a way so as to put the shapes in this pair out of collision.  It could serve as an
		 * approximation for a contact normal, I suppose.
		 * 
		 * @param[in] shapeID This is expected to be one of the IDs of the two shapes in this collision status pair.  This is the shape to be moved.
		 * @return The translation needed to move the given shape out of collision is returned; zero if the given shape is not a member of this pair.
		 */
		Vector3 GetSeparationDelta(ShapeID shapeID) const;

		/**
		 * Return the length of the separation delta.  This is zero if there is no collision.
		 */
		double GetSeparationDeltaLength() const;

		/**
		 * Return a point approximating the center of overlap, if any, between the two shapes in this collision status pair.
		 * It is left undefined if the pair are not actually in collision.  It could serve as an approximation for a contact
		 * point, I suppose.
		 */
		const Vector3& GetCollisionCenter() const { return this->collisionCenter; }

		/**
		 * If this is a valid collision status pair, then tell the caller if the two shapes
		 * involved are actually in collision (or overlap) with one another.
		 */
		bool AreInCollision() const { return this->inCollision; }

	public:
		/**
		 * This is used internally so that we can re-use code comparing A against B in the case of B against A.
		 */
		void FlipContext();

		bool inCollision;				///< Are the shapes in this pair thought to be in collision/overlapping?
		Vector3 collisionCenter;		///< This is an approximate center of the overlap region between the two shapes, if they are thought to be in collision; undefined, otherwise.
		Vector3 separationDelta;		///< This is a minimal translation delta that, if added to shape A or subtracted from shape B, will get them into a state of at most touching.  It is undefined if the shapes are not thought to be in collision.

	private:
		uint64_t revisionNumberA;		///< This cache entry was calculated when shape A was at this revision number.
		uint64_t revisionNumberB;		///< This cache entry was calculated when shape B was at this revision number.
		const Shape* shapeA;			///< This is the first shape in the collision pair.  Order doesn't matter.
		const Shape* shapeB;			///< This is the second shape in the collision pair.  Again, order doesn't matter.
	};
}