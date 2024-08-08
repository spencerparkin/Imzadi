#pragma once

#include "Defines.h"
#include <math.h>
#include <vector>
#include <istream>
#include <ostream>

namespace Imzadi
{
	/**
	 * Instances of this class are members of a 3-dimensional euclidean vector space,
	 * and form a group over addition.
	 * 
	 * Vectors are oriented line-segments in space with arbitrary position, but definite direction.
	 * They're often used to with various interpretations, such as positions in space (when the tail
	 * is placed at origin), directions with magnitude (such as forces or torques), and so on.
	 * No interpretation is imposed here.  Note that algebrically, vectors can and are often thought
	 * of as 3x1 or 1x3 matrices.
	 */
	class IMZADI_API Vector3
	{
	public:
		Vector3()
		{
			this->x = 0.0;
			this->y = 0.0;
			this->z = 0.0;
		}

		Vector3(double x, double y, double z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		Vector3(const Vector3& vector)
		{
			this->x = vector.x;
			this->y = vector.y;
			this->z = vector.z;
		}

		virtual ~Vector3()
		{
		}

		void operator=(const Vector3& vector)
		{
			this->x = vector.x;
			this->y = vector.y;
			this->z = vector.z;
		}

		void operator+=(const Vector3& vector)
		{
			this->x += vector.x;
			this->y += vector.y;
			this->z += vector.z;
		}

		void operator-=(const Vector3& vector)
		{
			this->x -= vector.x;
			this->y -= vector.y;
			this->z -= vector.z;
		}

		Vector3 operator-() const
		{
			return Vector3(
				-this->x,
				-this->y,
				-this->z
			);
		}

		void operator*=(double scalar)
		{
			this->x *= scalar;
			this->y *= scalar;
			this->z *= scalar;
		}

		void operator/=(double scalar)
		{
			this->x /= scalar;
			this->y /= scalar;
			this->z /= scalar;
		}

		/**
		 * Tell the caller if an Inf of NaN exists in the components of this vector.
		 * 
		 * @return True is returned if the vector is valid; false, otherwise.
		 */
		bool IsValid() const
		{
			if (::isnan(this->x) || ::isinf(this->x))
				return false;

			if (::isnan(this->y) || ::isinf(this->y))
				return false;

			if (::isnan(this->z) || ::isinf(this->z))
				return false;

			return true;
		}

		/**
		 * Tell the caller if this vector is of non-zero length.
		 */
		bool IsNonZero() const
		{
			return this->x != 0.0 || this->y != 0.0 || this->z != 0.0;
		}

		/**
		 * Tell the caller if this vector is of zero length.
		 */
		bool IsZero() const
		{
			return this->x == 0.0 && this->y == 0.0 && this->z == 0.0;
		}

		/**
		 * Set the x, y and z components of this vector.  This method is provided merely
		 * for convenience as the said components are made public in this class.
		 * 
		 * @param[in] x This is how far we go along the x-axis.
		 * @param[in] y This is how far we go along the y-axis.
		 * @param[in] z This is how far we go along the z-axis.
		 */
		void SetComponents(double x, double y, double z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		/**
		 * Get the x, y and z components of this vector.  This method is provided merely
		 * for convenience and consistency as the said components are made public in this class.
		 * 
		 * @param[out] x This is how far we go along the x-axis.
		 * @param[out] y This is how far we go along the y-axis.
		 * @param[out] z This is how far we go along the z-axis.
		 */
		void GetComponents(double& x, double& y, double& z) const
		{
			x = this->x;
			y = this->y;
			z = this->z;
		}

		/**
		 * Return the euclidean distance from tip to tail of this vector using
		 * a generalization of the Pythagorean Theorem.
		 */
		double Length() const
		{
			return ::sqrt(this->Dot(*this));
		}

		/**
		 * Return the square length of this vector.
		 */
		double SquareLength() const
		{
			return this->Dot(*this);
		}

		/**
		 * Return the dot-product (or inner-product) of this vector and the given vector.  Note that this operation is commutative.
		 * Therefore, there is no loss in generality to describe the operation as follows.  Putting the vectors
		 * tail-to-tail, the first vector is orthogonally projected down onto the second vector.  The result is
		 * then the length of the second vector multiplied by the length of this projection.
		 */
		double Dot(const Vector3& vector) const
		{
			return(
				this->x * vector.x +
				this->y * vector.y +
				this->z * vector.z
			);
		}

		/**
		 * Return this vector after having calculated it to be a vector mutually orthogonal to the given to vectors using the right-hand rule.
		 * The length of this vector will be equal to the area of the parallelagram formed by the two given vectors put tail-to-tail.
		 * Of course, this also means that if the two given vectors are parallel (point in the same direction or opposite directions), then
		 * the result is zero.  Note that by the right-hand rule, this type of product does not commute.  In other words, it matters which
		 * vector comes first and which comes second.
		 * 
		 * Of course, this is not to be confused with the outer-product, because we do not produce a blade of higher grade here.
		 * 
		 * @param[in] vectorA This first vector to be taken in the cross product.
		 * @param[in] vectorB The second vector to be taken in the cross product.
		 * @return A reference to this vector is returned for method-call chaining.
		 */
		Vector3& Cross(const Vector3& vectorA, const Vector3& vectorB)
		{
			this->x = vectorA.y * vectorB.z - vectorA.z * vectorB.y;
			this->y = vectorA.z * vectorB.x - vectorA.x * vectorB.z;
			this->z = vectorA.x * vectorB.y - vectorA.y * vectorB.x;

			return *this;
		}

		/**
		 * Calculate and return the cross product between this vector and the given vector, in that order.
		 * For more information, see the other overload.
		 * 
		 * @param[in] vector The second vector taken in the cross product.
		 * @return The result of the product is returned.
		 */
		Vector3 Cross(const Vector3& vector) const
		{
			Vector3 result;
			result.Cross(*this, vector);
			return result;
		}

		/**
		 * Return a vector in the same direction as this, but of unit-length.
		 */
		Vector3 Normalized() const;

		/**
		 * Rescale this vector to be of unit-length.  Of course, the direction is unchanged.
		 * 
		 * @param[out] length This is an optional parameter that, if given, will contain the length of the vector prior to normalization.
		 * @return True is returned on success; false, otherwise.  Failure can occur if the vector is zero or numerically too short in length.
		 */
		bool Normalize(double* length = nullptr);

		/**
		 * Calculate and return the vector that is the orthogonal projection of this vector
		 * onto the given unit-length vector.
		 * 
		 * @param[in] unitVector This is the vector upon which to project this vector.  It is assumed to be of unit-length.  The result is left undefined if this is not the case.
		 * @return The orthogonal projection is returned.  This vector is left untouched.
		 */
		Vector3 ProjectedOnto(const Vector3& unitVector) const;

		/**
		 * Calculate and return the vector that is the orthogonal rejection of this vector
		 * from the given unit-length vector.
		 * 
		 * @param[in] unitVector This is the vector from which to reject this vector.  It is assumed to be of unit-length.  The result is left undefined if this is not the case.
		 * @return The orthogonal rejection is returned.  This vector is left untouched.
		 */
		Vector3 RejectedFrom(const Vector3& unitVector) const;

		/**
		 * Caluclate and return the vector that is the rotation of this vector about the
		 * given unit-length vector.  Placing the vectors tail-to-tail, the tip of this
		 * vector is rotated about an axis determined by the given vector and the given angle.
		 * Looking at the axis vector while it is pointed at you, a positive angle of rotation
		 * corresponds to a counter-clock-wise rotation of this vector's head; a clock-wise
		 * rotation if negative.
		 * 
		 * @param[in] unitAxis This is the axis about which to perform the rotation.  It is assumed to be of unit-length.  The result is left undefined if this is not the case.
		 * @param[in] angle This is the angle, in radians, determining the amount by which this vector is rotated.
		 * @return The rotated vector is returned.  This vector is left untouched.
		 */
		Vector3 Rotated(const Vector3& unitAxis, double angle) const;

		/**
		 * Set this vector as one orthogonal (perpendicular) to the given vector.
		 * Exactly which vector we calculate here is defined only so far as to say
		 * that it will be non-zero and orthogonal to the given vector, provided
		 * the given vector is non-zero.  A vector is typically chosen to give the
		 * best numerical stability.
		 * 
		 * This method is useful when the caller wants a vector perpendicular to
		 * the given vector, and just doesn't care what vector it is, as long as
		 * it's perpendicular.
		 * 
		 * @param[in] vector A vector perpendicular to this vector will be chosen.
		 */
		void SetAsOrthogonalTo(const Vector3& vector);

		/**
		 * Tell the caller if the given point is approximately equal to this point.
		 * 
		 * @param[in] point This is the point to check against this point.
		 * @param[in] tolerance The points are approximately equal if they are within this distance of one another.
		 * @return True is returned if the points are approximately the same point; false, otherwise.
		 */
		bool IsPoint(const Vector3& point, double tolerance = 0.0) const;

		/**
		 * Tell the caller if this point is approximately equal to any of the given points.
		 * 
		 * @param[in] pointArray This is the set of points to check against this point.
		 * @param[in] tolerance A point is approximately equal to this one if it is within this distance of it.
		 * @return True is returned if any given point is approximately the same as this point; false, otherwise.
		 */
		bool IsAnyPoint(const std::vector<Vector3>& pointArray, double tolerance = 0.0) const;

		/**
		 * Write this vector to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this vector from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

		/**
		 * Calculate and return the angle between this vector and the given vector.
		 * Note that we assume that _both_ this vector and the given vector are of
		 * unit length.  If this is not the case, then we leave our result as undefined.
		 * 
		 * @param[in] unitVector This is a unit-length vector against which this vector is measured.
		 * @return The angle between this vector and the given vector is returned in radians.
		 */
		double AngleBetween(const Vector3& unitVector) const
		{
			return ::acos(this->Dot(unitVector));
		}

		/**
		 * Calculate and return the angle between this vector and the given vector,
		 * but in the range [0,2pi], not just [0,pi].  This is done using the given
		 * normal, which is assumed to be a vector mutual perpendicular to this vector
		 * (assumed to be of unit-length) and the given unit vector (also assumed to
		 * be of unit-length.)  See the return value description.
		 * 
		 * @param[in] unitVector The angle between this instances vector and this given vector is returned.
		 * @param[in] unitNormal The two vectors we're comparing are thought to be in a plane having this normal.
		 * @return The angle (in radians) needed to rotate this vector CCW in the plane of the given unit-normal to the other given unit-vector is returned.
		 */
		double AngleBetween(const Vector3& unitVector, const Vector3& unitNormal) const;

		/**
		 * Set this vector as the linear interpolation of the two given vectors by the given amount.
		 * 
		 * @param[in] vectorA This is the result when alpha is zero.
		 * @param[in] vectorB This is the result when alpha is one.
		 * @param[in] alpha This is the amount by which to linearly interpolate.  It is typically in [0,1], but this is not a requirement.
		 * @return A reference to this vector is returned for method-call chaining.
		 */
		Vector3& Lerp(const Vector3& vectorA, const Vector3& vectorB, double alpha);

		/**
		 * Set this vector as the spherical-linear interpolation of the two given vectors by the
		 * given amount.  If the two given vectors are not of unit-length, then we leave the
		 * result as undefined.  We also assume here that the two given vectors are not parallel
		 * with one another.  That is, facing in the same or opposite directions.  If this assumption
		 * is not met, then we again leave the result undefined.
		 * 
		 * @param[in] unitVectorA This is the result when alpha is zero.
		 * @param[in] unitVectorB This is the result when alpha is one.
		 * @param[in] alpha This is the amount by which to interpolate.  It is typically in [0,1], but this is not necessary.
		 * @return A reference to this vector is returned for method-call chaining.
		 */
		Vector3& Slerp(const Vector3& unitVectorA, const Vector3& unitVectorB, double alpha);

		/**
		 * Return a copy of this vector translated toward the given vector by the
		 * given amount, but without moving passed the given vector.
		 * 
		 * @param[in] vector This is the target we're trying to reach.
		 * @param[in] stepSize This is the distance we move unless it's too far.  This must be non-negative.
		 * @return A copy of this vector moved toward the given vector is returned.
		 */
		Vector3 MoveTo(const Vector3& vector, double stepSize) const;

	public:
		double x, y, z;
	};

	/**
	 * Calculate and return the sum of the two given vectors.  This operation commutes.
	 * Geometrically, the tip of one vectors is placed at the tail of the other.  The result
	 * is then found as the vector stretching from the tail of the first vector to the tip
	 * of the second.
	 */
	inline Vector3 operator+(const Vector3& vectorA, const Vector3& vectorB)
	{
		return Vector3(
			vectorA.x + vectorB.x,
			vectorA.y + vectorB.y,
			vectorA.z + vectorB.z
		);
	}

	/**
	 * Calculate and return the difference between the two given vectors.  This operation does not commute.
	 * Geometrically, reverse the direction of the second vector, then add them as described in the addition
	 * version of this method.
	 */
	inline Vector3 operator-(const Vector3& vectorA, const Vector3& vectorB)
	{
		return Vector3(
			vectorA.x - vectorB.x,
			vectorA.y - vectorB.y,
			vectorA.z - vectorB.z
		);
	}
	
	inline Vector3 operator*(const Vector3& vector, double scalar)
	{
		return Vector3(
			vector.x * scalar,
			vector.y * scalar,
			vector.z * scalar
		);
	}

	inline Vector3 operator*(double scalar, const Vector3& vector)
	{
		return Vector3(
			vector.x * scalar,
			vector.y * scalar,
			vector.z * scalar
		);
	}

	inline Vector3 operator/(const Vector3& vector, double scalar)
	{
		return Vector3(
			vector.x / scalar,
			vector.y / scalar,
			vector.z / scalar
		);
	}

	/**
	 * This is a component-wise multiply of the given vectors, not to be mistaken
	 * with something like the outer product, or the geometric product.
	 */
	inline Vector3 operator*(const Vector3& vectorA, const Vector3& vectorB)
	{
		return Vector3(
			vectorA.x * vectorB.x,
			vectorA.y * vectorB.y,
			vectorA.z * vectorB.z
		);
	}

	inline bool operator==(const Vector3& vectorA, const Vector3& vectorB)
	{
		return
			vectorA.x == vectorB.x &&
			vectorA.y == vectorB.y &&
			vectorA.z == vectorB.z;
	}

	inline bool operator!=(const Vector3& vectorA, const Vector3& vectorB)
	{
		return
			vectorA.x != vectorB.x ||
			vectorA.y != vectorB.y ||
			vectorA.z != vectorB.z;
	}
}