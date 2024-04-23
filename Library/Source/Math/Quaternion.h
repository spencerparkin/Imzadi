#pragma once

#include "Defines.h"

namespace Collision
{
	class Vector3;
	class Plane;
	class Ray;

	/**
	 * Quaternions are hyper-complex numbers.  They're also rotors in geometric algebra.
	 * Unit-quaternions form a group under multiplication and are useful for representing
	 * rotations or orientations.  
	 */
	class COLLISION_LIB_API Quaternion
	{
	public:
		Quaternion();
		Quaternion(double w, double x, double y, double z);
		Quaternion(const Quaternion& quat);
		virtual ~Quaternion();

		void operator=(const Quaternion& quat);
		void operator+=(const Quaternion& quat);
		void operator-=(const Quaternion& quat);
		void operator*=(const Quaternion& quat);
		void operator/=(const Quaternion& quat);

		/**
		 * Make sure that no component of the quaternion is Inf and NaN.
		 * 
		 * @return True is returned if valid; false, otherwise.
		 */
		bool IsValid() const;

		/**
		 * Set this quaternion to be the multiplicative identity.
		 * This is also the identity rotation.
		 */
		void SetIdentity();

		/**
		 * Set the imaginary part of this quaternion to the given point, and
		 * set the real part of this quaternion to zero.
		 * 
		 * @param[in] point This is the point to use as the imaginary part.
		 */
		void SetPoint(const Vector3& point);

		/**
		 * Get the imaginary part of this quaternion as the returned point.
		 * 
		 * @return The imaginary part is returned as a point vector.
		 */
		Vector3 GetPoint() const;

		/**
		 * Set this quaternion to a unit-magnitude quaternion that, when
		 * used to apply a rotation, does so about the given axis by the
		 * given angle.
		 * 
		 * @param[in] unitAxis This is the axis of the rotation and must be of unit-length.  If it is not of unit-length, the result is left undefined.
		 * @param[in] angle This is the angle, in radians, determining the amount of rotation.
		 */
		void SetFromAxisAngle(const Vector3& unitAxis, double angle);

		/**
		 * Extract from this quaternion the axis/angle pair for the
		 * rotation it represents.  We assume here that this quaternion
		 * is of unit-magnitude.  If that is not the case, then the
		 * result is left undefined.
		 * 
		 * Note that the axis/angle pair you get from this is not necessarily
		 * the same axis/angle pair that could have been used to formulate
		 * this quaternion.  Quaternions and axis/angle pairs uniquely represent
		 * rotations up to sign.
		 * 
		 * @param[out] unitAxis This will hold the axis of rotation and will be of unit-length.
		 * @param[out] angle This will hold the angle, in radians, of rotation.
		 */
		void GetToAxisAngle(Vector3& unitAxis, double& angle) const;

		/**
		 * Calculate and return the square of the magnitude of this quaternion.
		 * 
		 * @return The magnitude square of this quaternion is returned.
		 */
		double SquareMagnitude() const;

		/**
		 * Calculate and return the magnitude of this quaternion.
		 * 
		 * @return The magnitude of this quaternion is returned.
		 */
		double Magnitude() const;

		/**
		 * Calculate and return the conjugate of this quaternion.
		 * 
		 * @return The conjugate is returned.  If this is a unit-quaternion, then this is also the inverse of the quaternion.
		 */
		Quaternion Conjugated() const;

		/**
		 * Calculate and return the inverse of this quaternion.
		 * 
		 * @return The multiplicative inverse is returned.  If this quaternion is zero, then the result is left undefined.
		 */
		Quaternion Inverted() const;

		/**
		 * Calculate and return the normalization of this quaternion.
		 * 
		 * @return This quaternion scaled to unit-magnitude is returned.  If the quaternion is zero, then the result is left undefined.
		 */
		Quaternion Normalized() const;

		/**
		 * Calculate and return the rotation of the given point by this quaternion.
		 * We assume here that this quaternion is of unit-magnitude.  If this is
		 * not the case, then we leave the result undefined.
		 * 
		 * @param[in] point This is the point to be rotated.
		 * @return The rotated point is returned as output, leaving the input untouched.
		 */
		Vector3 Rotate(const Vector3& point) const;

		/**
		 * Calculate and return the rotation of the given plane by this quaternion.
		 * We assume here that this quaternion is of unit-magnitude.  If this is
		 * not the case, then we leave the result undefined.
		 * 
		 * @param[in] plane This is the plane to be rotated.
		 * @return The rotated plane is returned as output, leaving the input untouched.
		 */
		Plane Rotate(const Plane& plane) const;

		/**
		 * Calculate and return the rotation of the given ray by this quaternion.
		 * We assume here that this quaternion is of unit-magnitude.  If this is
		 * not the case, then we leave the result undefined.
		 * 
		 * @param[in] ray This is the ray to be rotated.
		 * @return The rotated ray is returned as output, leaving the input untouched.
		 */
		Ray Rotate(const Ray& ray) const;

		double w, x, y, z;
	};

	COLLISION_LIB_API Quaternion operator+(const Quaternion& quatA, const Quaternion& quatB);
	COLLISION_LIB_API Quaternion operator-(const Quaternion& quatA, const Quaternion& quatB);
	COLLISION_LIB_API Quaternion operator*(const Quaternion& quatA, const Quaternion& quatB);
	COLLISION_LIB_API Quaternion operator/(const Quaternion& quatA, const Quaternion& quatB);
	COLLISION_LIB_API Quaternion operator*(const Quaternion& quat, double scalar);
	COLLISION_LIB_API Quaternion operator*(double scalar, const Quaternion& quat);
	COLLISION_LIB_API Quaternion operator/(const Quaternion& quat, double scalar);
}