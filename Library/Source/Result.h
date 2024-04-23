#pragma once

#include "Defines.h"

namespace Collision
{
	/**
	 * Derivatives of this class are collision query results, or, in other words,
	 * responses to collision queries.
	 */
	class COLLISION_LIB_API Result
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
}