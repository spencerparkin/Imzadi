#pragma once

#include "Vector3.h"

namespace Collision
{
	class Vector3;

	/**
	 * These are planes of infinite extent in 3D space and non necessarily containing the origin.
	 * If the stored normal is not of unit-length, then all methods are left undefined.
	 */
	class COLLISION_LIB_API Plane
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

	public:
		Vector3 center;
		Vector3 unitNormal;
	};
}