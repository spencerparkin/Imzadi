#pragma once

#include "Defines.h"
#include <stdint.h>

namespace Collision
{
	class AxisAlignedBoundingBox;

	typedef uint32_t ShapeID;

	/**
	 * Derivatives of this class represent all the kinds of shapes that the collision system supports.
	 * These are all the shapes that can collide with one another.
	 */
	class COLLISION_LIB_API Shape
	{
	public:
		Shape();
		virtual ~Shape();

		/**
		 * Any new derivatives of the Shape class should add its own type ID here.
		 */
		enum TypeID
		{
			SPHERE,
			BOX,
			CAPSULE,
			POLYGON
		};

		/**
		 * All Shape object derivatives return a type ID that's used when doing a look-up
		 * for a CollisionPairCalculator instance that can perform the intersection
		 * calculations involving this shape.
		 * 
		 * @return This shape's type ID is returned for intersection purposes.
		 */
		virtual TypeID GetShapeTypeID() const = 0;

		/**
		 * All shape instances have a unique shape ID that can be used to safely refer
		 * to a shape, whether it still exists in the system or has gone extinct.
		 * It is often needed in other API calls.
		 * 
		 * @return The shape's ID is returned for reference purposes.
		 */
		ShapeID GetShapeID() const;

		/**
		 * Calculate and return the smallest AABB containing this shape.
		 * 
		 * @param[out] boundingBox The AABB that best fits this shape in its current position and orientation.
		 */
		virtual void CalcBoundingBox(AxisAlignedBoundingBox& boundingbox) const = 0;

		/**
		 * Tell the caller if this collision shape has valid data.
		 * 
		 * @return True is returned if the shape is valid; false, otherwise.
		 */
		virtual bool IsValid() const = 0;

		/**
		 * Calculate and return the area or volume of this shape.
		 */
		virtual double CalcSize() const = 0;

		//virtual bool ContainsPoint(const Vector3& point) = 0;

		/**
		 * Free the memory used by the given shape.  You should never allocate or free a shape yourself,
		 * because the calling code may be using a different heap than that of the collision system.
		 * You should also never free a shape of which you do not have ownership.  In most cases, ownership
		 * of shapes is passed to the collision system when you add the shape to the collision world, at
		 * which point the system owns the memory, and you will likely never need to free it yourself.
		 * 
		 * @param[in] shape This is the shape who's memory is to be reclaimed.
		 */
		static void Free(Shape* shape);

	private:
		ShapeID shapeID;
		static ShapeID nextShapeID;
	};
}