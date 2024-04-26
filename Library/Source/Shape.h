#pragma once

#include "Defines.h"
#include "Math/Transform.h"
#include <stdint.h>

namespace Collision
{
	class AxisAlignedBoundingBox;
	class DebugRenderResult;

	typedef uint32_t ShapeID;

	/**
	 * Derivatives of this class represent all the kinds of shapes that the collision system supports.
	 * These are all the shapes that can collide with one another.  A shape is described in object
	 * space and is found in world space using an object-to-world transform.  The whole point of the
	 * collision system is to let the user create shapes in the world, move them around, and query
	 * to see which are in collision with which other shapes, and how.
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
		 * Calculate and return the smallest AABB containing this shape in world space.
		 * 
		 * @param[out] boundingBox The AABB that best fits this shape in its current position and orientation.
		 */
		virtual void CalcBoundingBox(AxisAlignedBoundingBox& boundingbox) const = 0;

		/**
		 * Tell the caller if this collision shape has valid data.  Overrides should
		 * call this base-class method.  This function is provided mainly for debugging
		 * purposes, and is not meant to be called in a production use-case.
		 * 
		 * @return True is returned if the shape is valid; false, otherwise.
		 */
		virtual bool IsValid() const;

		/**
		 * Calculate and return the area or volume of this shape.
		 */
		virtual double CalcSize() const = 0;

		// TODO: virtual bool ContainsPoint(const Vector3& point) = 0;

		/**
		 * Overrides of this method should populate the given DebugRenderResult class instance
		 * with lines of a consistent color for the purpose of debug visualzation of the collision system.
		 * The lines added to the given result should be in world-space.
		 */
		virtual void DebugRender(DebugRenderResult* renderResult) const = 0;

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

		/**
		 * Set this shape's transform taking it from object space to world space.
		 */
		void SetObjectToWorldTransform(const Transform& objectToWorld);

		/**
		 * Get this shape's transform taking it from object space to world space.
		 */
		const Transform& GetObjectToWorldTransform() const;

		/**
		 * Get the inverse of this shape's transform taking it from object space to world space.
		 * The returned transform will take points from world space to object space.
		 */
		const Transform& GetWorldToObjectTransform() const;

		/**
		 * Set the color of this shape when it is drawn for debugging/visualization purposes.
		 * 
		 * @param[in] color Here, the x, y and z components are used for red, green and blue, respectively.
		 */
		void SetDebugRenderColor(const Vector3& color) { this->debugColor = color; }

		/**
		 * Get the color of this shape that is used for debug drawing purposes.
		 * 
		 * @return The color is returned as a vector, the x, y and z components representing red, green and blue, respectively.
		 */
		const Vector3& GetDebugRenderColor() const { return this->debugColor; }

	private:
		ShapeID shapeID;
		static ShapeID nextShapeID;

	protected:
		Transform objectToWorld;
		mutable Transform worldToObject;
		mutable bool worldToObjectValid;
		Vector3 debugColor;
	};
}