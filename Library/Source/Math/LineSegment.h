#pragma once

#include "Vector3.h"

namespace Collision
{
	class Plane;

	/**
	 * These are simply pairs of points, and we imagine a line connecting them.
	 * The line-segment does not have any direction, like a vector, although
	 * the user could infer one based on the order of the points, and there are
	 * a few methods that depend on this order.  A degenerate line-segment
	 * (a point) is considered invalid.
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
		 * Calculate and return the point on this line segment that is closest
		 * to the given point.
		 */
		Vector3 ClosestPointTo(const Vector3& point) const;

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

		/**
		 * Linearly interpolate between the two points of this line-segment by
		 * the given amount.  Of course, the order of the points in this case
		 * matters.
		 * 
		 * @param[in] lambda This is the interpolation parameter.
		 * @return If the interpolation parameter is in [0,1], we return here a point on the line-segment.  A point on the line containing this line-segment is returned.
		 */
		Vector3 Lerp(double lambda) const;

	public:
		Vector3 point[2];
	};
}