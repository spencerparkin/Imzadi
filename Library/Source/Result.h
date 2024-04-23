#pragma once

#include "Defines.h"
#include "Math/LineSegment.h"
#include <vector>

namespace Collision
{
	/**
	 * Derivatives of this class are collision query results; or, in other words,
	 * responses to collision queries.  There is not necessarily a one-to-one correspondance
	 * between derivatives of this class and that of the Query class, but there typically
	 * is a Result class derivative per Query class derivative.  When a query is made and
	 * a result given, the user will know, based on the query that was made, the derivative
	 * of the Result class to which the Result class pointer can be safely cast.  Each
	 * Query class derivative should document the Result class derivative that it uses.
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

	/**
	 * A result of a DebugRenderQuery class query, this class contains all of
	 * wire-frame drawing information the user can use to visualize (render)
	 * the collision system as far as what was requested in the query.
	 */
	class COLLISION_LIB_API DebugRenderResult : public Result
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
			LineSegment line;	//< The user should render this line's geometry, given here in world space coordinates.
			Vector3 color;		//< The user should render the line with this color to differentiate it from other elements of the collision world.
		};

		/**
		 * Return the list of RenderLine structure instances that the
		 * caller can use to visualize the state of the collision system.
		 * Of course, no drawing code is supported by the collision system.
		 * It's up to the caller to render the data.
		 */
		const std::vector<RenderLine>& GetRenderLineArray() const { return *this->renderLineArray; }

		/**
		 * Add a line to be rendered to the debug render result.
		 * This is typically just used internally by the collision system.
		 */
		void AddRenderLine(const RenderLine& renderLine);

		/**
		 * Allocate and return a new DebugRenderResult object.
		 */
		static DebugRenderResult* Create();

	private:
		std::vector<RenderLine>* renderLineArray;
	};
}