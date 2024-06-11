#pragma once

#include "Defines.h"

namespace Imzadi
{
	class Vector3;

	/**
	 * This class provides an alternative way to locate points in 3D space using
	 * a radius and two angles.  Here, the poles of the spheres in question are
	 * on the positive or negative Y-axes.  All sphere centers are at origin.
	 */
	class IMZADI_API SphericalCoords
	{
	public:
		SphericalCoords();
		SphericalCoords(double radius, double longitudeAngle, double latitudeAngle);
		SphericalCoords(const SphericalCoords& sphericalCoords);
		virtual ~SphericalCoords();

		void operator=(const SphericalCoords& sphericalCoords);

		/**
		 * Set these spherical coordinates to those where the given vector's
		 * head is located when its tail is at origin.
		 *
		 * @param[in] vector Calculate this vector's spherical coordinates.
		 */
		void SetFromVector(const Vector3& vector);

		/**
		 * Get the vector's who's head is at this spherical coordinate's location
		 * when the vector's tail is at origin.
		 *
		 * @return The point at this spherical coordinate's location is returned.
		 */
		Vector3 GetToVector() const;

		/**
		 * Set this set of coordinates to the linear interpolation of two given coordinates
		 * by the given amount.  This is similar to a spherical-linear interpolation of vectors.
		 * 
		 * @param[in] coordsA This is the result when alpha is zero.
		 * @param[in] coordsB This is the result when alpha is one.
		 * @param[in] alpha This is the amount by which to linearly interpolate.
		 * @return A reference to these coords is returned for method-call chaining.
		 */
		SphericalCoords& Lerp(const SphericalCoords& coordsA, const SphericalCoords& coordsB, double alpha);

	public:
		double radius;				///< This is the point's distance from the origin.
		double longitudeAngle;		///< This is the longitudinal angle (in radians) of the point.  Two pi gets you around the sphere.
		double latitudeAngle;		///< This is the latitudinal angle (in radians) of the point.  One pi gets you from pole to pole.
	};
}