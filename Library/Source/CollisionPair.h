#include "Defines.h"
#include "Math/Vector3.h"
#include <stdint.h>

namespace Collision
{
	class Shape;

	typedef uint64_t CollsionPairKey;

	/**
	 * These are instances of known collisions (intersections) between two shapes in the system.
	 * The primary purpose of these is to cache intersection information for efficiency purposes.
	 * For example, if work is done to determine that shape A intersects with shape B, then this
	 * relationship commutes, and we can say that B interects with A, and clearly we would not
	 * want to redo all the calculations that were necessary to determine this, and to determine
	 * how the shapes are in collision.  Like all caching schemes, care must be taken to invalidate
	 * cached items when they are no longer a reflection of reality.  Collision pair objects become
	 * invalid whenever a shape of the collision pair is translated or reoriented.
	 */
	class COLLISION_LIB_API CollisionPair
	{
	public:
		CollisionPair();
		virtual ~CollisionPair();

		//static CollisionPairKey MakePairKey(const Shape* shapeA, const Shape* shapeB);

		struct Contact
		{
			Vector3 point;				///< This is the point of contact on the shape.  It is on the surface of the shape.
			Vector3 normal;				///< This is the surface normal of the shape at the contact point.
			double penetrationDepth;	///< This is the distance below the contact point, in the opposite direction of the surface normal, to which the shape has been penetrated.
		};

	private:
		Shape* shapeA;
		Shape* shapeB;

		Contact contactA;
		Contact contactB;
	};
}