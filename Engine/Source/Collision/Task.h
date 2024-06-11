#pragma once

#include "Defines.h"
#include <stdint.h>

namespace Collision
{
	typedef uint32_t TaskID;

	class Thread;

	class COLLISION_LIB_API Task
	{
	public:
		Task();
		virtual ~Task();

		/**
		 * Derivatives of the Task class must impliment this method.
		 * Note that tasks execute on the collision thread.
		 *
		 * @param thread The task may use the thread object to perform its work.
		 */
		virtual void Execute(Thread* thread) = 0;

		/**
		 * Get the unique identifier for this task.  These IDs are used as safe
		 * handles the caller can use instead of pointer than can potentially
		 * go stale.  Further, a C-pointer to a task may not be thread-safe
		 * to access anyway.
		 */
		TaskID GetTaskID() const { return this->taskID; }

		/**
		 * This function can and should be used to free any command or query
		 * allocated by the collision system API.
		 * 
		 * @param task This is the task who's memory is to be reclaimed.
		 */
		static void Free(Task* task);

	private:
		TaskID taskID;
		static TaskID nextTaskID;
	};
}