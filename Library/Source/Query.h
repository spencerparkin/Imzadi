#pragma once

#include "Task.h"
#include "Math/Ray.h"
#include <stdint.h>

namespace Collision
{
	class Result;

	/**
	 * This class and its derivatives are the means by which the collision system
	 * user makes inqueries into the collision state of a given collision shape, or,
	 * in general, into any state of the collision system.
	 * Of course, it would not make sense for a collision system to be continuously
	 * checking, for every shape, if it is (or is not) in collision with every other
	 * shape of the system.  It is up to the user to tell _us_ what shapes it cares
	 * about by querying for collision results involving those desired shapes.
	 */
	class COLLISION_LIB_API Query : public Task
	{
	public:
		Query();
		virtual ~Query();

		virtual void Execute(Thread* thread) override;

		virtual Result* ExecuteQuery(Thread* thread) = 0;
	};

	/**
	 * This is a query that can be made once per frame to produce a result
	 * that can be used for debug rendering purposes, such as to visualize
	 * all of the collision shapes in the system.  See the DebugRenderResult class.
	 * This is a useful feature when trying to debug your application.  It is
	 * not intended to be used in the production-case of your application.
	 */
	class COLLISION_LIB_API DebugRenderQuery : public Query
	{
	public:
		DebugRenderQuery();
		virtual ~DebugRenderQuery();

		virtual Result* ExecuteQuery(Thread* thread) override;

		/**
		 * Tell the collision system what parts of it you want to visualize.
		 * 
		 * @param[1] drawFlags This is some OR-ing of the COLL_SYS_DRAW_FLAG_* defines.
		 */
		void SetDrawFlags(uint32_t drawFlags) { this->drawFlags = drawFlags; }

		/**
		 * Return this query's configure draw flags.
		 * 
		 * @return An OR-ing of some combination of the COLL_SYS_DRAW_FLAG_* defines is returned.
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
	class COLLISION_LIB_API RayCastQuery : public Query
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

	//ShapeQuery -- What other shapes is this shape in collision with and how?
	// Note that if the underlying system is smart, work to see if A collides with B will not be duplicated to find that B collides with A.
}