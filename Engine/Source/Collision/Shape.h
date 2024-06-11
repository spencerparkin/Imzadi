#pragma once

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/AxisAlignedBoundingBox.h"
#include <stdint.h>
#include <atomic>
#include <ostream>
#include <istream>

namespace Imzadi
{
	class DebugRenderResult;
	class BoundingBoxNode;
	class ShapeCache;

	typedef uint64_t ShapeID;

	/**
	 * Derivatives of this class represent all the kinds of shapes that the collision system supports.
	 * These are all the shapes that can collide with one another.  A shape is described in object
	 * space and is found in world space using an object-to-world transform.  The whole point of the
	 * collision system is to let the user create shapes in the world, move them around, and query
	 * to see which are in collision with which other shapes, and how.
	 * 
	 * Note that users create these on the main thread and then hand them over to the collision thread.
	 * The time until the shapes are handed over is the only time the user can directly mutate the shape.
	 * Once it's handed over, the user can only safely refer to a shape by ID, and can only safely mutate
	 * that shape indirectly through the use of collision system commands.
	 * 
	 * For now, only convex collision shapes are supported, but there is no requirement that this be
	 * the case in the overall design of the system.  An arbitrary mesh that is not necessarily convex
	 * would be the most flexible type of collision shape I can think of, but is presently well beyond
	 * the capabilities of this system.
	 */
	class IMZADI_API Shape
	{
		friend class BoundingBoxTree;
		friend class BoundingBoxNode;
		friend class ShapeCache;

	public:
		/**
		 * Construct a new shape.  If temporary, it means that the shape is not
		 * intended for long-term use in the collision system.  For example, the
		 * shape is being allocated on the callstack.  If not temporary, the shape
		 * is assigned an ID that can be used to safely refer to it on any thread.
		 */
		Shape(bool temporary);

		/**
		 * Destruct the shape.  It's important this is virtual, of course.
		 */
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
		 * It is often needed in other API calls.  The number zero is reserved as an
		 * ID no shape will ever have, and represents the absense of a shape, or "null",
		 * if you will, in contexts where a shape ID can show up.
		 * 
		 * @return The shape's ID is returned for reference purposes.
		 */
		ShapeID GetShapeID() const;

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

		/**
		 * This is an alternative to doing a dynamic cast.
		 */
		template<typename T>
		T* Cast()
		{
			return (T::StaticTypeID() == this->GetShapeID()) ? (T*)this : nullptr;
		}

		/**
		 * This is the constant version of our dynamic cast alternative.
		 */
		template<typename T>
		const T* Cast() const
		{
			return (T::StaticTypeID() == this->GetShapeID()) ? (const T*)this : nullptr;
		}

		/**
		 * Allocate and return a clone of this shape.
		 */
		virtual Shape* Clone() const = 0;

		/**
		 * Abandon this shape's internals and clone those of the given shape.
		 * Any override of this method should call this base method.
		 * 
		 * @param[in] shape This is assumed to be of the same derivative type as this shape.  A dynamic cast prevents crashing.
		 */
		virtual bool Copy(const Shape* shape);

		/**
		 * During insertion into the AABB tree, if splitting is allowed, and splitting is
		 * supported by this shape, then this method can be used to split this shape across
		 * box boundaries so that it can be inserted deeper into the tree.  The original
		 * shape is deleted and the split parts of the shape live on.
		 * 
		 * @param[in] plane This is the plane across which this shape is split.
		 * @param[out] shapeBack This will be assigned the part of the shape on the back side of the given plane.
		 * @param[out] shapeFront This will be assigned the part of the shape on the front side of the given plane.
		 * @return False is returned by default, indicating that no split can occur.  True is returned and a split is performed otherwise.
		 */
		virtual bool Split(const Plane& plane, Shape*& shapeBack, Shape*& shapeFront) const;

		/**
		 * Tell the caller if this shape contains the given point as
		 * a surface point or interior point.
		 * 
		 * @param[in] point This is the point to test against this shape.
		 * @return True is returned if the given point is contained within this shape; false, otherwise.
		 */
		virtual bool ContainsPoint(const Vector3& point) const = 0;

		/**
		 * Overrides of this method should populate the given DebugRenderResult class instance
		 * with lines of a consistent color for the purpose of debug visualzation of the collision system.
		 * The lines added to the given result should be in world-space.
		 */
		virtual void DebugRender(DebugRenderResult* renderResult) const = 0;

		/**
		 * Overrides of this method should perform a ray-cast operation of the
		 * given world-space ray against this shape in world space.  If the ray
		 * originates within the shape, then false should be returned.
		 * 
		 * @param[in] ray This is the ray to cast against this shape.
		 * @param[out] alpha This is distance along the given ray from its origin to the surface point, if any, where this shape is hit; undefined if no hit occurs.
		 * @param[out] unitSurfaceNormal This should be the normal to the shape's surface at the point of ray impact, if any; undefined if not hit occurs.
		 * @return True is returned if the given ray hits this shape; false, otherwise.
		 */
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const = 0;

		/**
		 * Overrides should serialize this shape to the given stream.  Note that they
		 * should call this base-class method before providing their own implimentation.
		 * 
		 * @param[out] stream Write a binary representation of the shape to this stream.
		 * @return True should be returned if the shape is successfully dumped; false, otherwise.
		 */
		virtual bool Dump(std::ostream& stream) const;

		/**
		 * Overrides should deserialize this shape from the given stream.  Note that they
		 * should call this base-class method before providing their own implimentation.
		 * 
		 * @param[in] stream Read a binary representation of this shape from this stream.
		 * @return True should be returned if the shape is successfully restored; false, otherwise.
		 */
		virtual bool Restore(std::istream& stream);

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
		 * This is a shape class factory creating the shape corresponding to the given type.
		 */
		static Shape* Create(TypeID typeID);

		/**
		 * Set this shape's transform taking it from object space to world space.
		 * Note that to keep things simple, all calculations assume that there
		 * is no shear or scale in the matrix part of the given transform.  In other
		 * words, we assume that it's a rigid-body transform.  If this is not the
		 * case, then we leave the results of all calculations undefined.
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

		/**
		 * Return a number that should change every time this collision shape is changed in any
		 * way that would effect its collision status.
		 */
		uint64_t GetRevisionNumber() const { return this->revisionNumber; }

		/**
		 * Increment a number that should change every time this collision shape is changed in any
		 * way that could effect its collision status.  This is done internally where appropriate,
		 * and there is no need to do it externally.  Redundant calls to this are fine.
		 */
		void BumpRevisionNumber() { this->revisionNumber++; }

		/**
		 * Tell the caller if this shape is bound to a node in the bounding box tree.
		 * This also means that the shape is, as of last insertion, still considered
		 * to be within the bounds of the collision world.  Of course, the bounding
		 * box of this shape may not actually be within the collision world.
		 */
		bool IsBound() const { return this->node != nullptr; }

	private:

		ShapeID shapeID;							///< This is a unique identifier that can be used to safely refer to this node on any thread.
		static std::atomic<ShapeID> nextShapeID;	///< This is the ID of the next shape to be allocated by the system.
		BoundingBoxNode* node;						///< This is the node of the bounding-box tree that contains this shape.
		mutable ShapeCache* cache;					///< This pointer should never be accessed directly by methods of this class or any of its derivatives.  Rather, the GetCache method should always be used.

	protected:

		/**
		 * This method should be used internally by any other class method to get
		 * access to the ShapeCache member pointer.
		 */
		ShapeCache* GetCache() const;

		/**
		 * Derivatives must override this to provide a ShapeClass derivative allocation.
		 */
		virtual ShapeCache* CreateCache() const = 0;

	protected:

		Transform objectToWorld;	///< A shape is described in object space and then realized in world space using this transform.
		Vector3 debugColor;			///< This color is used to render the shape for debugging purposes.
		uint64_t revisionNumber;	///< This is used in the collision cache mechanism.  Any change to the shape should bump this number.
	};

	/**
	 * This class holds information that is redundant about a shape, or that can be
	 * gleaned from the defining characteristics of the shape.  The purpose here is
	 * to prevent us from recalculating this information every time we need it, and
	 * to provide a formal caching mechanism we can use to manage it.  Also, while
	 * all shapes share some common attributes we'd like to cache, some require
	 * additional information, and that information can be stored in a derivative
	 * of this class.
	 * 
	 * Methods of the associated shape class need to be careful to invalidate this
	 * cache class instance whenever necessary.
	 * 
	 * Another way to think of this class is as a way of enforcing single-source of truth.
	 * For example, the object-to-world transform is a single source of truth, while
	 * we let the world-to-object transform always be a function of that truth.  This way,
	 * we're never uncertain about what is current and what is possibly not.  If both
	 * transforms were the source of truth, then our code has to work hard everywhere to
	 * keep them in sync, and it's too error-prone.
	 * 
	 * Lastly, (as if this isn't already super long-winded and over-the-top), making this
	 * its own class creates a clear separation in the data between what is a defining
	 * characteristic of a shape and what is gleanable/additional or redundant information
	 * about the shape.
	 */
	class ShapeCache
	{
	public:
		ShapeCache();
		virtual ~ShapeCache();

		/**
		 * Overrides of this method should call this base method as well as provide a
		 * means of updating its own members as well as members of this base class that
		 * cannot be generally updated, such as the bounding-box.
		 */
		virtual void Update(const Shape* shape);

	public:
		bool isValid;						///< If true, the other members of this class should be a reflection of reality; false, otherwise.
		Transform worldToObject;			///< This should be calculated as the inverse of this shape's object-to-world transform.
		AxisAlignedBoundingBox boundingBox;	///< This should be calculated as the smallest AABB that contains this shape.
	};
}