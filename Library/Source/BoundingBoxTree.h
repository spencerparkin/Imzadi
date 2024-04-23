#pragma once

#include "Defines.h"

namespace Collision
{
	/**
	 * This class facilitates the broad-phase of collision detection.
	 */
	class COLLISION_LIB_API BoundingBoxTree
	{
	public:
		BoundingBoxTree();
		virtual ~BoundingBoxTree();
	};
}