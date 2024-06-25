#pragma once

#include "Defines.h"

namespace Imzadi
{
	/**
	 * TODO: Write this.
	 * 
	 * I'm hoping to do some rigid-body dynamics here with spheres and boxes at the
	 * very least.  This is a tall order, because the math is hard, and the collision
	 * is a close second.  I do have a collision system, but I'm not sure it's up
	 * to the task.  Just a few thoughts on implimentations...  There are two parts
	 * to getting this working: 1) simulating unconstrained rigid-bodies moving through
	 * space (not too terribly hard), and then 2) adding constraints.  This second part
	 * can be broken down into two problem: 1) separating all rigid bodies after integration,
	 * and 2) resolving collisions using impulses.  These are both hard, but can be delt
	 * with independently.  Resting contact is possibly a monkey-wrench in all of this.
	 */
	class IMZADI_API PhysicsSystem
	{
	public:
		PhysicsSystem();
		virtual ~PhysicsSystem();
	};
}