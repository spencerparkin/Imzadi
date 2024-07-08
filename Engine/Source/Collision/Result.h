#pragma once

#include "Defines.h"
#include "Shape.h"
#include "Math/LineSegment.h"
#include "Math/Transform.h"
#include "Reference.h"
#include <vector>
#include <string>

namespace Imzadi
{
	class AxisAlignedBoundingBox;

	/**
	 * Derivatives of this class are collision query results; or, in other words,
	 * responses to collision queries.  There is not necessarily a one-to-one correspondance
	 * between derivatives of this class and that of the Query class, but there typically
	 * is a Result class derivative per Query class derivative.  When a query is made and
	 * a result given, the user will know, based on the query that was made, what derivatives
	 * of the Result class to which the query result pointer can be possibly cast.  Each
	 * Query class derivative should document the Result class derivatives that it uses.
	 * Users of the system should perform a dynamic cast on the returned Result class pointer.
	 * Any query can possibly return an instance of the ErrorResult class.
	 */
	class IMZADI_API Result
	{
	public:
		Result();
		virtual ~Result();

		/**
		 * Use this function to free any given collision result once you've
		 * had a chance to process it on the main thread.
		 * 
		 * @param result This is the result who's memory is to be reclaimed.
		 */
		static void Free(Result* result);
	};

	/**
	 * This result is returned by any query that requirse a yes/no answer.
	 */
	class IMZADI_API BoolResult : public Result
	{
	public:
		BoolResult();
		virtual ~BoolResult();

		/**
		 * Get at the boolean result of the query.
		 */
		bool GetAnswer() const { return this->answer; }

		/**
		 * This is used by the query to set the result.
		 */
		void SetAnswer(bool answer) { this->answer = answer; }

		/**
		 * Allocate and return a new BoolResult instance.
		 */
		static BoolResult* Create();

	private:
		bool answer;
	};

	/**
	 * A result of a DebugRenderQuery class query, this class contains all of
	 * wire-frame drawing information the user can use to visualize (render)
	 * the collision system as far as what was requested in the query.
	 */
	class IMZADI_API DebugRenderResult : public Result
	{
	public:
		DebugRenderResult();
		virtual ~DebugRenderResult();

		/**
		 * An instance of this structure are used to generate wire-frame
		 * representations of shapes and other things in the collision system.
		 */
		struct RenderLine
		{
			LineSegment line;	///< The user should render this line's geometry, given here in world space coordinates.
			Vector3 color;		///< The user should render the line with this color to differentiate it from other elements of the collision world.
		};

		/**
		 * Return the list of RenderLine structure instances that the
		 * caller can use to visualize the state of the collision system.
		 * Of course, no drawing code is supported by the collision system.
		 * It's up to the caller to render the data.
		 */
		const std::vector<RenderLine>& GetRenderLineArray() const { return this->renderLineArray; }

		/**
		 * Add a line to be rendered to the debug render result.
		 * This is typically just used internally by the collision system.
		 */
		void AddRenderLine(const RenderLine& renderLine);

		/**
		 * Add 12 lines for the given AABB.
		 * 
		 * @param[in] box This is the box to render as wire-frame.
		 * @param[in] color This is the color to use for the box.
		 */
		void AddLinesForBox(const AxisAlignedBoundingBox& box, const Vector3& color);

		/**
		 * Allocate and return a new DebugRenderResult object.
		 */
		static DebugRenderResult* Create();

	private:
		std::vector<RenderLine> renderLineArray;
	};

	/**
	 * An instance of this class is returned as the result of a ray-cast query
	 * using the RayCastQuery class.
	 */
	class RayCastResult : public Result
	{
	public:
		RayCastResult();
		virtual ~RayCastResult();

		/**
		 * This structure organizes the characteristics of a ray-cast hit against a shape in the collision world.
		 */
		struct HitData
		{
			ShapeID shapeID;			///< This is the ID of the collision shape that was hit by the ray, if any.  It is zero if no hit occured.
			Vector3 surfacePoint;		///< This is the point on the surface of the shape where the ray hit it.
			Vector3 surfaceNormal;		///< This is the normal to the surface of the shape where it was hit.
			double alpha;				///< Mainly used for internal purposes, this is the distance from ray origin along the ray to the hit point.
		};

		/**
		 * Get the particular of the ray-cast result in the returned structure.
		 * If the returned hit-data has zero for the shape ID, then the ray did
		 * not hit any shape in the collision world.
		 */
		const HitData& GetHitData() const { return this->hitData; }

		/**
		 * This is used internally to set the hit-data on the ray-cast result object.
		 */
		void SetHitData(const HitData& hitData) { this->hitData = hitData; }

		/**
		 * Allocate and return a new RayCastResult instance.
		 */
		static RayCastResult* Create();

	private:
		HitData hitData;
	};

	/**
	 * Instances of this class are results of the ObjectToWorldQuery.
	 */
	class IMZADI_API ObjectToWorldResult : public Result
	{
	public:
		ObjectToWorldResult();
		virtual ~ObjectToWorldResult();

		static ObjectToWorldResult* Create();

	public:
		Transform objectToWorld;
	};

	class ShapePairCollisionStatus;

	/**
	 * Instances of this class are results of collision queries, and simply consist
	 * of a set of collision pairs.  See the ShapePairCollisionStatus class for
	 * more information.
	 */
	class IMZADI_API CollisionQueryResult : public Result
	{
	public:
		CollisionQueryResult();
		virtual ~CollisionQueryResult();

		/**
		 * Allocate and return a new instance of the CollisionQueryResult class.
		 */
		static CollisionQueryResult* Create();

		/**
		 * This is used internally to populate the query result.
		 */
		void AddCollisionStatus(ShapePairCollisionStatus* collisionStatus);

		/**
		 * Get this result's set of ShapePairCollisionStatus class instances.
		 * Each pair will be a valid collision pair between two shapes, one of
		 * which is the shape specified in the original collision query.
		 */
		const std::vector<Reference<ShapePairCollisionStatus>>& GetCollisionStatusArray() const { return this->collisionStatusArray; }

		/**
		 * This is used internally to set the ID of the shape in question.
		 */
		void SetShapeID(ShapeID shapeID) { this->shapeID = shapeID; }

		/**
		 * Get the ID of the shape that was used in the collision query.
		 */
		ShapeID GetShapeID() const { return this->shapeID; }

		/**
		 * This is used internally to set the object-to-world transform of the shape in question.
		 */
		void SetObjectToWorldTransform(const Transform& objectToWorld) { this->objectToWorld = objectToWorld; }

		/**
		 * Get the object-to-world transform of the shape that was used in the collision query.
		 */
		const Transform& GetObjectToWorldTransform() const { return this->objectToWorld; }

		/**
		 * Find and return the collusion status pair of this result with the largest penetration depth.
		 * 
		 * @return Null is returned here if there is no collision status pair in a colliding state.
		 */
		const ShapePairCollisionStatus* GetMostEgregiousCollision() const;

		/**
		 * Calculate and return the average separation delta for all collision status pairs
		 * of this result that are in the collision status.
		 * 
		 * @param shapeID This will determine the direction of the separation deltas.  The direction will move the shape with this ID away from the other shape of the pair.
		 */
		Vector3 GetAverageSeparationDelta(ShapeID shapeID) const;

	private:
		std::vector<Reference<ShapePairCollisionStatus>> collisionStatusArray;	///< This is the set of collisions involving the collision query's shape.
		ShapeID shapeID;			///< For convenience, this holds the ID of the shape in question that was the subject of the collision query.
		Transform objectToWorld;	///< For convenience, this is the object-to-world transform of the shape in question at the time of query.
	};
}