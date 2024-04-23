#pragma once

#include "Task.h"

namespace Collision
{
	class Result;

	/**
	 * This class and its derivatives are the means by which the collision system
	 * user makes inqueries into the collision state of a given collision shape.
	 * Of course, it would not make sense for a collision system to be continuously
	 * checking, for every shape, if it is (or is not) in collision with every other
	 * shape of the system.  It is up to the user to tell _us_ what shapes it cares
	 * about by querying for collision results involving those shapes.
	 */
	class COLLISION_LIB_API Query : public Task
	{
	public:
		Query();
		virtual ~Query();

		virtual void Execute(Thread* thread) override;

		virtual Result* ExecuteQuery(Thread* thread) = 0;
	};

	//RayCastQuery -- What shape, if any, does this ray hit, and where?
	//ShapeQuery -- What other shapes is this shape in collision with and how?
	// Note that if the underlying system is smart, work to see if A collides with B will not be duplicated to find that B collides with A.
	//DebugRenderQuery -- Returns something that can draw the collision shapes using an interface with a user-provided back-end.
}