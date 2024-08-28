#pragma once

#include "Vector3.h"

namespace Imzadi
{
	class Plane;
	class AxisAlignedBoundingBox;

	/**
	 * These are simply pairs of points, and we imagine a line connecting them.
	 * The line-segment does not have any direction, like a vector, although
	 * the user could infer one based on the order of the points, and there are
	 * a few methods that depend on this order.  A degenerate line-segment
	 * (a point) is considered invalid.
	 */
	class IMZADI_API LineSegment
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
		 * Calculate and return the square length of this line segment.
		 * It is faster to compare square lengths instead of lengths, because
		 * we forgo a square root.
		 */
		double SquareLength() const;

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
		 * @param[in] alpha This is the interpolation parameter.
		 * @return If the interpolation parameter is in [0,1], we return here a point on the line-segment.  A point on the line containing this line-segment is returned.
		 */
		Vector3 Lerp(double alpha) const;

		/**
		 * This method is meant to be the inverse of the @ref Lerp method.
		 * 
		 * @param[in] point This is a point thought to be on the line segment (which isn't necessarily the case.)
		 * @param[out] alpha If the given point is on this line-segment (within the given tolerance), then this is its interpolation parameter, which can be outside [0,1].
		 * @return True is returned here if the given point is on the line of this line-segment; false, otherwise.
		 */
		bool Alpha(const Vector3& point, double& alpha, double tolerance = 1e-6) const;

		/**
		 * Tell the caller if the given point is on this line segment.
		 * 
		 * @param[in] point This is the point to test against this line segment.
		 * @param[out] isInterior If given, this is populated with a boolean value indicating whether the given point is interior to the line-segment, or is one of the terminal points.
		 * @param[in] tolerance We're looking to see if the given point is within at most this distance from the line-segment.
		 */
		bool ContainsPoint(const Vector3& point, bool* isInterior = nullptr, double tolerance = 1e-6) const;

		/**
		 * Tell the caller if the given point is interior to this line segment.
		 */
		bool ContainsInteriorPoint(const Vector3& point, double tolerance = 1e-6) const;

		/**
		 * Set this line segment as the shortest line segment connecting the two given line segments.
		 * Note that we can fail here in cases where there is no single shortest connector.  For example,
		 * consider two distinct yet parallel line-segments, side-by-side.  Many shortest connectors
		 * exist in this case, and we fail in this case.
		 * 
		 * @param[in] lineSegmentA The first point of this segment will be on this given line segment.
		 * @param[in] lineSegmentB The second point of this segment will be on this given line segment.
		 * @return True is returned on success; false, otherwise, and this line segment is left undefined.
		 */
		bool SetAsShortestConnector(const LineSegment& lineSegmentA, const LineSegment& lineSegmentB);

		/**
		 * This method handles the fail-case of the @ref SetAsShortestConnector function involving
		 * two line segments.  It is possible to produce a degenerate line-segment here.
		 */
		bool SetAsAnyShortestConnector(const LineSegment& lineSegmentA, const LineSegment& lineSegmentB);

		/**
		 * Set this line segment as the shortest line segment connecting the given line segment
		 * and the given axis-aligned box.  Note that we can fail here in cases where there is no
		 * single shortest connector.  Note that we can return a degenerate line segment here in
		 * the case that the given line segment intersects the given plane.
		 * 
		 * @param[in] lineSegment The first point of this line segment will be on this given line segment.
		 * @param[in] box The second point of this line segment will be on this given box.
		 * @return True is returend on success; false, otherwise, and this line segment is left undefined.
		 */
		bool SetAsShortestConnector(const LineSegment& lineSegment, const AxisAlignedBoundingBox& box);

		/**
		 * Set this line segment as the shortest line segment connecting the given line segment
		 * and the given plane.  Note that we can fail here in cases where there is no single
		 * shortest connector.
		 * 
		 * @param[in] lineSegment The first point of this line segment will be on this given line segment.
		 * @param[in] plane The second point of this line segment will be on this given plane.
		 * @return True is returend on success; false, otherwise, and this line segment is left undefined.
		 */
		bool SetAsShortestConnector(const LineSegment& lineSegment, const Plane& plane);

		/**
		 * Return the difference between the vertices of this line segment.
		 * Of course, this operation will depend on the order of the points.
		 * 
		 * @return Return the second point minus the first point of this line segment.
		 */
		Vector3 GetDelta() const;

		/**
		 * Swap the first and second points of this line-segment.  In many
		 * situations, it doesn't matter which is which, but sometimes it
		 * does matter when using certain method of this class.
		 */
		void Reverse();

		/**
		 * Write this line segment to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this line segment from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		Vector3 point[2];
	};
}