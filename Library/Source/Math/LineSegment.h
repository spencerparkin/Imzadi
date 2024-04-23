#pragma once

#include "Vector3.h"

namespace Collision
{
	class Plane;

	/**
	 * These are simply pairs of points, and we imagine a line connecting them.
	 * The line-segment does not have any direction, like a vector, although
	 * the user could infer one based on the order of the points.  A degenerate
	 * line-segment (a point) is considered invalid.
	 */
	class COLLISION_LIB_API LineSegment
	{
	public:
		LineSegment();
		LineSegment(const Vector3& pointA, const Vector3& pointB);
		virtual ~LineSegment();

		bool IsValid() const;

		/**
		 * Calculate and return the length of this line segment.
		 */
		double Length() const;

		/**
		 * Calculate and return the shortest distance from the given point
		 * to any point on this line segment.
		 */
		double ShortestDistanceTo(const Vector3& point) const;

		/**
		 * Calculate and return the shortest distance between any point
		 * on this line segment with any point on the given line segment.
		 */
		double ShortestDistanceTo(const LineSegment& lineSegment) const;

		/**
		 * Calculate and return the shortest distance between any point
		 * on this line segment with any point on the given plane.
		 */
		double ShortestDistanceTo(const Plane& plane) const;

	public:
		Vector3 point[2];
	};
}