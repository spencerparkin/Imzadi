#pragma once

#include "Vector3.h"

namespace Collision
{
	class Plane;

	/**
	 * A ray here is described by a point and unit-length vector pair.  The ray starts
	 * at the point and proceeds in the direction of the vector without bound.
	 * Points on the ray are defined as the point plus a non-negative scalar multiple
	 * of the ray direction, usually referred to as the alpha value.  If the stored
	 * ray direction is not of unit-length, then all methods are left undefined.
	 */
	class COLLISION_LIB_API Ray
	{
	public:
		Ray();
		Ray(const Vector3& point, const Vector3& unitDirection);
		Ray(const Ray& ray);
		virtual ~Ray();

		void operator=(const Ray& ray);
		
		/**
		 * Tell the caller if this is a valid ray.  Apart from Inf or NaN,
		 * we're invalid if our direction is not unit-length.
		 * 
		 * @param[in] tolerance The length of the direction vector is checked to be within this tolerance of one.
		 * @return True is returned if valid; false, otherwise.
		 */
		bool IsValid(double tolerance = 1e-7) const;

		/**
		 * Calculate and return the point on the ray at the given alpha value.
		 * 
		 * @param[in] alpha This is the length along the ray at which the desired point is found and returned.  It is typically non-negative.
		 * @return The point on the ray at the given alpha value is returned.
		 */
		Vector3 CalculatePoint(double alpha) const;

		/**
		 * Calculate and return the alpha value for the given ray-point.
		 * 
		 * @param[in] rayPoint This is assumed to be a point on the ray.
		 * @return The alpha value of the given point is returned.  What's returned is left undefined if the given ray-point is not actually a point on the ray.
		 */
		double CalculateAlpha(const Vector3& rayPoint) const;

		/**
		 * Calculate and return the alpha value for the ray point of
		 * this ray intersecting the given plane.
		 * 
		 * @param[in] plane The plain against which to cast the ray.
		 * @return The said alpha value is returned.  We lave the result undefined if there is no intersection.
		 */
		double CastAgainst(const Plane& plane) const;

		/**
		 * Calculate and return the alpha value for the ray point of
		 * this ray intersecting the given plane, if any.
		 * 
		 * @param[in] plane The plain against which to cast the ray.
		 * @param[out] alpha The said alpha value is returned in this.
		 * @return True is returned if the ray hits the plane; false, otherwise.
		 */
		bool CastAgainst(const Plane& plane, double& alpha) const;

	public:
		Vector3 origin;
		Vector3 unitDirection;
	};
}