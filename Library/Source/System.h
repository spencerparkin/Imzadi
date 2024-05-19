#pragma once

#include "Defines.h"
#include "Task.h"
#include "Shape.h"
#include "Math/AxisAlignedBoundingBox.h"

// TODO: Just ran across this: https://github.com/kevinmoran/GJK/blob/master/GJK.h
//       He has some good references here on GJK and other collision detecdtion algorithms.
//       I'm probably doing everything here in a really stupid way.

namespace Collision
{
	class Shape;
	class Command;
	class Query;
	class Result;
	class Thread;

	/**
	 * This is the main interface to the collision system.  An application will typically instantiate
	 * just one of these for its purposes, but it could instantiate more.  A single instance manages
	 * a collection of collidable shapes in a given pre-defined region.  Users can send commands to
	 * reposition or reorient the shapes, and make queries to, for example, see what shapes a given
	 * shape is in collision with.  This interface is designed to be asynchronous with the hope that
	 * efficiency is gained through parallelism.
	 * 
	 * See the GetError function for error handling and error information.
	 * 
	 * Note that the collision system does not resolve collisions or satisfy constraints based on
	 * any kind of physics-based simulation.  Those details are left to the user.  Our only goal
	 * here is to facilitate the complex problem of collision detection.  Physics is a seperate
	 * problem.
	 */
	class COLLISION_LIB_API System
	{
	public:
		System();
		virtual ~System();

		/**
		 * Initialize the collision system.  You must call this before using the system.
		 * 
		 * @param collisionWorldExtents This is an AABB defining the scope of the entire collision world/system.  All shapes that will ever be created must fit in this box.
		 * @return True is returned on success; false, otherwise.
		 */
		bool Initialize(const AxisAlignedBoundingBox& collsionWorldExtents);

		/**
		 * Shutdown the collision system.  You should call this before your program exits.
		 * 
		 * @return True is returned on success; false, otherwise.
		 */
		bool Shutdown();

		/**
		 * Add a collision shape to the collision system.  All shapes tracked by the system can be queried or commanded.
		 * The given shape should be allocated using an API call.  Allocators are typically static methods of the desired
		 * Shape class derivative.
		 * 
		 * @param shape This is a pointer to the Shape object derivative.  Ownership of the memory is taken by the system.
		 * @param flags This is an OR-ing of the COLL_SYS_ADD_FLAG_* flags.
		 * @return A handle to the collision shape is returned.  Use it to reference the shape in any command or query.
		 */
		ShapeID AddShape(Shape* shape, uint32_t flags);

		/**
		 * Remove the collision shape from the collision system having the given shape ID.  This does nothing if
		 * the shape ID is invalid, except generate an error.
		 * 
		 * @param shapeID This is a thread-safe handle to the shape you want to remove.
		 */
		void RemoveShape(ShapeID shapeID);

		/**
		 * Remove all collision shapes from the system.  Typically you'd call this when you want to wipe
		 * everything clean and rebuild your world of collidable shapes.
		 */
		void Clear();

		/**
		 * Send a command to the collision system; typically to translate or rotate a collision shape.
		 * Call an API function to allocate a collisoin command.  You should never allocate it yourself.
		 * Allocators are typically static methods of the desired Command class derivative.
		 * 
		 * @param[in] command This is a pointer to a Command object derivative.  Ownership of the memory is taken by the system.
		 * @return True is returned on success; false, otherwise.
		 */
		bool IssueCommand(Command* command);

		/**
		 * Make a collision query against the system.  Call an API function to allocate the given query.  You should
		 * never allocate it yourself.  Allocators are typically static methods of the desired Query command
		 * class derivative, but you can also use the Create and Free method of the System class.
		 * 
		 * Note that queries are always processed in the same order that they are made.
		 * 
		 * @param[in] query This is a pointer to Query object derivative.  Ownership of the memory is taken by the system.
		 * @param[out] taskID A handle to the query is returned.  Use it in a call to ObtainQueryResult.
		 * @return True is returned on success; false, otherwise.
		 */
		bool MakeQuery(Query* query, TaskID& taskID);

		/**
		 * Get the result of a collision query, if it is available.  Rather than call this
		 * function with the possibility that the result is not yet available, it probably makes
		 * more sense to issue all needed queries, then at a later point, stall the system until
		 * all queries are complete.  See the FlushAllTasks function.
		 * 
		 * @param[in] taskID This is a handle to the collision query that was made; what is returned by the MakeQuery function.
		 * @return A Result object derivative is returned.  The caller takes ownership of the memory.  See the Free function.
		 */
		Result* ObtainQueryResult(TaskID taskID);

		/**
		 * Free the memory associated with the given object.  Note that no heap-allocated object
		 * used by the collision system should ever be created or destroyed by the collision system user
		 * using the standard new and delete operators.  Rather, they should be created or destroyed by
		 * the collision system user using the Create or Free methods, respectively.  This is because the
		 * user's heap and the collision system's heap may not be the same.  The collision system might
		 * even impliment it's own high-performance heap to boost efficiency.
		 */
		template<typename T>
		void Free(T* object)
		{
			T::Free(object);
		}

		/**
		 * Allocate and construct a new object of the given type.  This can be a Shape, a Query, a Command, etc.
		 * See the corresponding Free function.
		 */
		template<typename T>
		T* Create()
		{
			return T::Create();
		}

		/**
		 * Stall until all collision tasks (queries or commands) are complete.  Once this function has returned,
		 * all previously issued commands will have been executed, and every call to ObtainQueryResult with a
		 * valid query handle will succeed.  In other words, no command or query is pending or in flight once
		 * this call returns with success.
		 * 
		 * @return True is returned on success; false, otherwise.
		 */
		bool FlushAllTasks();

		/**
		 * Dump the current physics world (all the shapes) to the given file.  This is mainly used
		 * for debugging purposes.
		 * 
		 * @param[in] fileName This is a string containing the fully-qualified path to where the data should be dumped.
		 * @return True is returned on success; false, otherwise.
		 */
		bool DumpToFile(const std::string& fileName);

		/**
		 * Restore a previously dumped physics world (all the shapes) from the given file.  This is mainly
		 * used for debugging purposes.
		 * 
		 * @param[in] fileName This is a string containing the fully-qualified path to where the data can be restored from.
		 * @return True is returned on success; false, otherwise.
		 */
		bool RestoreFromFile(const std::string& fileName);

	private:
		Thread* thread;
	};
}