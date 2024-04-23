#pragma once

#include "Defines.h"
#include <math.h>

namespace Collision
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
	class COLLISION_LIB_API Vector3
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
		 * Return the euclidean distance from tip to tail of this vector using
		 * a generalization of the Pythagorean Theorem.
		 */
		double Length() const
		{
			return ::sqrt(this->Dot(*this));
		}

		/**
		 * Return the dot-product of this vector and the given vector.  Note that this operation is commutative.
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
		 * @param[in] vectorA This first vector to be taken in the cross product.
		 * @param[in] vectorB The second vector to be taken in the cross product.
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
}