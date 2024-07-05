#pragma once

#include "Task.h"
#include "Math/Ray.h"
#include "Shape.h"
#include <stdint.h>

namespace Imzadi
{
	class Result;

	/**
	 * This class and its derivatives are the means by which the collision system
	 * user makes inqueries into the collision state of a given collision shape; or,
	 * in general, into any state of the collision system.
	 * Of course, it would not make sense for a collision system to be continuously
	 * checking, for every shape, if it is (or is not) in collision with every other
	 * shape of the system.  It is up to the user to tell _us_ what shapes it cares
	 * about by querying for collision results involving those desired shapes.
	 */
	class IMZADI_API Query : public Task
	{
	public:
		Query();
		virtual ~Query();

		virtual void Execute(Thread* thread) override;

		virtual Result* ExecuteQuery(Thread* thread) = 0;
	};

	/**
	 * This is the base class for all queries about a particular shape.
	 */
	class IMZADI_API ShapeQuery : public Query
	{
	public:
		ShapeQuery();
		virtual ~ShapeQuery();

		/**
		 * Set the shape ID of the shape about which to query.
		 */
		void SetShapeID(ShapeID shapeID) { this->shapeID = shapeID; }

		/**
		 * Get the shape ID of the shape about which to query.
		 */
		ShapeID GetShapeID() const { return this->shapeID; }

	protected:
		ShapeID shapeID;
	};

	/**
	 * This is a query that can be made once per frame to produce a result
	 * that can be used for debug rendering purposes, such as to visualize
	 * all of the collision shapes in the system.  See the DebugRenderResult class.
	 * This is a useful feature when trying to debug your application.  It is
	 * not intended to be used in the production-case of your application.
	 */
	class IMZADI_API DebugRenderQuery : public Query
	{
	public:
		DebugRenderQuery();
		virtual ~DebugRenderQuery();

		virtual Result* ExecuteQuery(Thread* thread) override;

		/**
		 * Tell the collision system what parts of it you want to visualize.
		 * 
		 * @param[1] drawFlags This is some OR-ing of the IMZADI_DRAW_FLAG_* defines.
		 */
		void SetDrawFlags(uint32_t drawFlags) { this->drawFlags = drawFlags; }

		/**
		 * Return this query's configure draw flags.
		 * 
		 * @return An OR-ing of some combination of the IMZADI_DRAW_FLAG_* defines is returned.
		 */
		uint32_t GetDrawFlags() const { return this->drawFlags; }

		/**
		 * Allocate and construct a new DebugRenderQuery object.
		 */
		static DebugRenderQuery* Create();

	private:
		uint32_t drawFlags;
	};

	/**
	 * Use this class to submit a ray-cast query against the entire physics world.
	 */
	class IMZADI_API RayCastQuery : public Query
	{
	public:
		RayCastQuery();
		virtual ~RayCastQuery();

		/**
		 * Perform the ray-cast query on the collision thread.
		 */
		virtual Result* ExecuteQuery(Thread* thread) override;

		/**
		 * Specify the ray to use in this ray-query.
		 * 
		 * @param[in] ray This ray will be cast against the collision world in this query.
		 */
		void SetRay(const Ray& ray) { this->ray = ray; }

		/**
		 * The ray being used in this query is returned.
		 */
		const Ray& GetRay() { return this->ray; }

		/**
		 * Allocate and return a new RayCastQuery instance.
		 */
		static RayCastQuery* Create();

	private:
		Ray ray;
	};

	/**
	 * Query for a collision shape's object-to-world transform.
	 * 
	 * An ObjectToWorldResult class instance is returned by this query.
	 */
	class IMZADI_API ObjectToWorldQuery : public ShapeQuery
	{
	public:
		ObjectToWorldQuery();
		virtual ~ObjectToWorldQuery();

		/**
		 * Extract the shape's object-to-world transform.
		 */
		virtual Result* ExecuteQuery(Thread* thread) override;

		/**
		 * Allocate and return a new instance of the ObjectToWorldQuery class.
		 */
		static ObjectToWorldQuery* Create();
	};

	/**
	 * Perform a query into the collision status of a given collision shape.
	 * This is the main feature of the entire collision system!  It's all been
	 * leading up to this!  What other shapes, if any, is this shape presently
	 * in collision with, and how?  A CollisionQueryResult class instance is
	 * returned by this query.
	 * 
	 * Note that, for the sake of simplicity, we do nothing to account for tunneling
	 * here, nor do we try to solve for the moment of impact between two shapes.
	 * In other words, time is not a variable that we consider here at all.
	 * Rather, what we do is determine which shapes are touching or in non-zero overlap
	 * with this query's shape, and then the minimum linear movement necessary on the part of
	 * either shape in order to get them into a state where they are at most touching one another.
	 * The recommended direction this either shape could move in order to separate
	 * may not correspond to the direction they may have moved to come in contact with one another,
	 * or have any basis in physical reality, such as a bounce reflection trajectory.  However,
	 * the said direction might be approximated as a contact normal.
	 * 
	 * A more sophisticated query may be written later that does try to take time into account,
	 * and prevent tunneling.
	 */
	class IMZADI_API CollisionQuery : public ShapeQuery
	{
	public:
		CollisionQuery();
		virtual ~CollisionQuery();

		/**
		 * Perform the collision query, calculating and collecting all
		 * collision pairs involving the given shape.
		 */
		virtual Result* ExecuteQuery(Thread* thread) override;

		/**
		 * Create an instance of the CollisionQuery class.
		 */
		static CollisionQuery* Create();

		// TODO: Maybe add flag here indicating that the caller doesn't care about
		//       any calculations of how to resolve the collision; they just want
		//       to know if a collision is occurring.  The query code doesn't have
		//       to obey this flag if everything it has to do already to determine
		//       a collision produced all the info anyway.  But there are cases where
		//       we can know if things are colliding without going further.
	};

	/**
	 * Use this query to find out if a given shape is still within
	 * the bounds of the collision world.  If it's not, then it will
	 * not have a place in the bounding-box tree, and so it can't
	 * collide with anything else in the world.
	 */
	class IMZADI_API ShapeInBoundsQuery : public ShapeQuery
	{
	public:
		ShapeInBoundsQuery();
		virtual ~ShapeInBoundsQuery();

		/**
		 * See if the shape has a node in the bounding box tree.
		 */
		virtual Result* ExecuteQuery(Thread* thread) override;

		/**
		 * Create an instance of the ShapeInBoundsQuery class.
		 */
		static ShapeInBoundsQuery* Create();
	};
}