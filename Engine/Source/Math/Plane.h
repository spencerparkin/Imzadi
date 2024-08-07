#pragma once

#include "Vector3.h"
#include <vector>

namespace Imzadi
{
	class Vector3;
	class LineSegment;

	/**
	 * These are planes of infinite extent in 3D space and non necessarily containing the origin.
	 * If the stored normal is not of unit-length, then all methods are left undefined.
	 */
	class IMZADI_API Plane
	{
	public:
		/**
		 * The default plane is at the origin and orthogonal to the z-axis.
		 */
		Plane();

		/**
		 * Make a plane that contains the given point and is orthogonal to the given normal.
		 * A point and normal uniquely determine a plane.  Note that the stored point or "center"
		 * will be the point on the plane closest to the origin.
		 * 
		 * @param[in] point A point on the desired plane.
		 * @param[in] unitNormal The normal to the desired plane.  If not of unit-length, all methods are left undefined.
		 */
		Plane(const Vector3& point, const Vector3& unitNormal);

		/**
		 * Make this plane a copy of the given plane.
		 */
		Plane(const Plane& plane);

		virtual ~Plane();

		/**
		 * Make this plane a copy of the given plane.
		 */
		void operator=(const Plane& plane);

		/**
		 * Tell the caller if this plane is valid.  Apart from the presents of Inf or NaN,
		 * we're invalid if our normal is not of unit-length.  (Yes, representing a plane
		 * as 4 floats precludes this possability, but I'm just trying to be consistent here.)
		 *
		 * @param[in] tolerance The length of the normal is checked to be within this tolerance of one.
		 * @return True is returned if the plane is valid; false, otherwise.
		 */
		bool IsValid(double tolerance = 1e-7) const;

		/**
		 * Return true if and only if this plane is the same as the given plane
		 * up to the given tolerance.  Note that the result here is left undefined
		 * if this plane and the given one are not both normalized.
		 * 
		 * @param[in] plane This is the plane compared with this class instance.
		 * @param[in] epsilon The plane centers (points on the planes closest to origin) must be within this distance, and the normals within this angle of one another.
		 * @return True is returned if the given plane and this one are the same.
		 */
		bool IsPlane(const Plane& plane, double epsilon = 1e-6);

		/**
		 * Without changing the plane represented by this class instance, make
		 * sure the normal is of unit-length and that the plane center is the
		 * point on the plane closest to origin.
		 * 
		 * @return True is returned on success; false, otherwise.
		 */
		bool Normalize();

		/**
		 * Calculate and return the shortest signed distance of the given point
		 * to this plane.  The absolute value of the result is the distance of
		 * the given point to the plane.  The sign of the result indicates which
		 * side of the plane the point is on.  If positive, the point is on the
		 * same side to which the normal points; negative, otherwise.
		 * 
		 * @param point This is the point in question.
		 * @return The said signed distance is returned.
		 */
		double SignedDistanceTo(const Vector3& point) const;

		/**
		 * We consider the front side of a plane to be the side to which the plane normal points.
		 */
		enum Side
		{
			FRONT,
			BACK,
			NEITHER
		};

		/**
		 * Tell the caller which side of the plane the given point lies.
		 * 
		 * @param[in] point This is the point to test against this plane.
		 * @param[in] planeThickness This is the thickness, if any, to given the plane, so that a point can approximately lie on the plane.
		 * @return FRONT, BACK or NEITHER is returned.  See the Side enum.
		 */
		Side GetSide(const Vector3& point, double planeThickness = 0.0) const;

		/**
		 * Tell the caller if all given points lie on the given side.
		 * 
		 * @param[in] pointArray This is the set of points to test against this plane.
		 * @param[in] side This is what side of the plane the caller wants to test.
		 * @return True is returned if all given points lie on the given side; false, otherwise.
		 */
		bool AllPointsOnSide(const std::vector<Vector3>& pointArray, Side side) const;

		/**
		 * Tell the caller if any one given point lies on the given side.
		 * 
		 * @param[in] pointArray This is the set of points to test against this plane.
		 * @param[in] side This is what side of the plane the caller wants to test.
		 * @return True is returned if any given point lies on the given side; false, otherwise.
		 */
		bool AnyPointOnSide(const std::vector<Vector3>& pointArray, Side side) const;

		/**
		 * Calculate and return the point on this plane that is closest
		 * to the given point.
		 * 
		 * @param[in] point This plane is compared agaisnt this given point.
		 * @return The point on this plane closest to the given point is returned.
		 */
		Vector3 ClosestPointTo(const Vector3& point) const;

		/**
		 * Calculate and return the point on this plane that is closest
		 * to the given point.
		 * 
		 * @param[in] lineSegment This plane is compared against this given line segment.
		 * @param[out] planePoint Our result is put into this variable.
		 * @return True is returned on success; false, otherwise.  We can fail if the line-segment is parallel to the plane.
		 */
		bool ClosestPointTo(const LineSegment& lineSegment, Vector3& planePoint) const;

		/**
		 * Calculate and return the shortest distance from this plane to the given line segment.
		 */
		double ShortestDistanceTo(const LineSegment& lineSegment) const;

		/**
		 * Write this plane to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this plane from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		Vector3 center;
		Vector3 unitNormal;
	};
}