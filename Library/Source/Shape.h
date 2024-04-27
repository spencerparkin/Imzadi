#pragma once

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/AxisAlignedBoundingBox.h"
#include <stdint.h>

namespace Collision
{
	class DebugRenderResult;
	class BoundingBoxNode;

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
		friend class BoundingBoxTree;
		friend class BoundingBoxNode;

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
		 * Derivatives must override this method to recalculate this shape's internal cache.
		 * See the Cache structure.  Derivatives must also be careful to invalidate this
		 * cache whenever necessary, and they should call this base method in their override.
		 */
		virtual void RecalculateCache() const;

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
		 * This calls the RecalculateCache() method if the cache is not currently valid.
		 * The cache is flagged as valid after this call.
		 */
		void RegenerateCacheIfNeeded() const;

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
		 * Return the smallest AABB containing this shape.
		 */
		const AxisAlignedBoundingBox& GetBoundingBox() const;

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

		ShapeID shapeID;				//< This is a unique identifier that can be used to safely refer to this node on any thread.
		static ShapeID nextShapeID;		//< This is the ID of the next shape to be allocated by the system.
		BoundingBoxNode* node;			//< This is the node of the bounding-box tree that contains this shape.

	protected:

		/**
		 * Any redundant data about the shape should be stored here.
		 */
		struct Cache
		{
			Transform worldToObject;			//< This should be calculated as the inverse of this shape's object-to-world transform.
			AxisAlignedBoundingBox boundingBox;	//< This should be calculated as the smallest AABB that contains this shape.
		};

		Transform objectToWorld;	//< A shape is described in object space and then realized in world space using this transform.
		Vector3 debugColor;			//< This color is used to render the shape for debugging purposes.
		mutable Cache cache;		//< This is cached data about the shape that can be gleaned as a function of the shape's defining characteristics.  The cache is used for efficiency purposes.
		mutable bool cacheValid;	//< This flag indicates whethere our cache is currently valid.  It becomes invalid whenever our object-to-world transform changes, or other defining characteristics of the shape.
	};
}