#pragma once

#include "Defines.h"
#include "Task.h"
#include "Shape.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "BoundingBoxTree.h"
#include <thread>
#include <mutex>
#include <list>
#include <semaphore>
#include <unordered_map>

namespace Collision
{
	class Task;
	class Result;
	class DebugRenderResult;

	/**
	 * This class impliments, and provides an interface to, the collision thread.
	 * It is not meant to be a user-facing class, but is solely used by the System
	 * class.  Users of the collision system API shouldn't even be able to get a
	 * pointer to this class, nor should they ever need one.  Note that some methods
	 * of this class are meant to be called only from the main thread, or only from
	 * the collision thread.
	 */
	class COLLISION_LIB_API Thread
	{
		friend class ExitThreadCommand;

	public:
		Thread(const AxisAlignedBoundingBox& collisionWorldExtents);
		virtual ~Thread();

		/**
		 * Kick-off this thread.
		 * 
		 * @return True is returned on success; false, otherwise.
		 */
		bool Startup();

		/**
		 * Gracefully terminate this thread from the main thread.
		 * 
		 * @return True is returned on success; false, otherwise.
		 */
		bool Shutdown();

		/**
		 * Send a task to this thread from the main thread.  The given task should
		 * be created using the collision system API.  Ownership of the memory is taken
		 * by this thread.  The caller should consider their pointer invalid
		 * once the call has returned.
		 * 
		 * @param[in] task This is the task (command or query) to be performed by this thread.
		 * @return A task ID is returned.  The caller can safely refer to this task in other API calls using this ID.
		 */
		TaskID SendTask(Task* task);

		/**
		 * Retrieve the result of a previously made query, if it's ready, from the main thread.
		 * After sending a bunch of queries, the user may wish to do some other work.  Once that
		 * work is done, a call to FlushAllTasks can be made, at which point, any call to this
		 * method should succeed with a valid task ID.
		 * 
		 * @param[in] taskID This is the task ID of the query that was previously made.
		 * @return A pointer to the query result, if any, is returned; null, otherwise.  The caller takes ownership of the memory and should free it when done.
		 */
		Result* ReceiveResult(TaskID taskID);

		/**
		 * Store a newly calculated result for the query of the given taskID.
		 * If a result is already stored for the given query, then it is replaced,
		 * and the other result is freed.
		 * 
		 * @param[in] result This is the result to store.
		 * @param[in] taskID This is a handle to the query.
		 */
		void StoreResult(Result* result, TaskID taskID);

		/**
		 * Add the given shape to the collision world.
		 * 
		 * @param shape This is the shape to add.
		 */
		void AddShape(Shape* shape);

		/**
		 * Remove the given shape from the collision world.
		 * 
		 * @param shapeID This is the shape ID of the shape to be removed from the collision system.
		 */
		void RemoveShape(ShapeID shapeID);

		/**
		 * Find and return the shape having the given shape ID.
		 * 
		 * @param[in] shapeID This is the ID of the shape to find within the collision world.
		 * @return If found, a pointer to the shape is returned; null, otherwise.
		 */
		Shape* FindShape(ShapeID shapeID);

		/**
		 * Wipe out all currently stored collision shapes.
		 */
		void ClearShapes();

		/**
		 * Produce a debug visualization of the collision system.
		 * 
		 * @param[out] renderResult This is populated with line-segments of various colors to produce a wire-frame rendering of the system.
		 * @param[in] drawFlags An OR-ing of the COLL_SYS_DRAW_FLAG_* defines is given here to determine what's produced in the result.
		 */
		void DebugVisualize(DebugRenderResult* renderResult, uint32_t drawFlags);

		/**
		 * Block until all pending tasks have been processed by this thread.
		 * This is not a busy wait, so it should not significantly consume any
		 * CPU resources.
		 */
		void WaitForAllTasksToComplete();

	private:

		/**
		 * This is the main thread-entry function.
		 * 
		 * @param thread This is the thread object representing the thread being executed.
		 */
		static void EntryFunc(Thread* thread);

		/**
		 * This is our thread implimentation.  All we do is consume and
		 * process thread tasks.
		 */
		void Run();

		/**
		 * Wipe out all currently queued tasks without processing them.
		 */
		void ClearTasks();

		/**
		 * Wipe out all currently stored results before they can be processed by the user.
		 */
		void ClearResults();

	private:
		BoundingBoxTree boxTree;
		bool signaledToExit;
		std::thread* thread;
		std::mutex* taskQueueMutex;
		std::list<Task*>* taskQueue;		// TODO: May want to replace this with a lock-free queue at some point.
		std::counting_semaphore<4096>* taskQueueSemaphore;
		std::mutex* resultMapMutex;
		std::unordered_map<TaskID, Result*>* resultMap;
		std::unordered_map<ShapeID, Shape*>* shapeMap;
		std::mutex* allTasksDoneMutex;
		std::condition_variable* allTasksDoneCondVar;
		bool allTasksDone;
	};
}