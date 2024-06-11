#pragma once

#include "Defines.h"
#include <math.h>
#include <istream>
#include <ostream>

namespace Collision
{
	/**
	 * Instances of this class are members of fa 2-dimensional euclidean vector space.
	 * Though this library focuses mainly on 3D space, 2-dimensional problems often
	 * arrise.
	 */
	class COLLISION_LIB_API Vector2
	{
	public:
		Vector2()
		{
			this->x = 0.0;
			this->y = 0.0;
		}

		Vector2(double x, double y)
		{
			this->x = x;
			this->y = y;
		}

		Vector2(const Vector2& vector)
		{
			this->x = vector.x;
			this->y = vector.y;
		}

		virtual ~Vector2()
		{
		}

		void operator=(const Vector2& vector)
		{
			this->x = vector.x;
			this->y = vector.y;
		}

		void operator+=(const Vector2& vector)
		{
			this->x += vector.x;
			this->y += vector.y;
		}

		void operator-=(const Vector2& vector)
		{
			this->x -= vector.x;
			this->y -= vector.y;
		}

		Vector2 operator-() const
		{
			return Vector2(
				-this->x,
				-this->y
			);
		}

		void operator*=(double scalar)
		{
			this->x *= scalar;
			this->y *= scalar;
		}

		void operator/=(double scalar)
		{
			this->x /= scalar;
			this->y /= scalar;
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

			return true;
		}

		/**
		 * Set the x and y components of this vector.  This method is provided merely
		 * for convenience as the said components are made public in this class.
		 *
		 * @param[in] x This is how far we go along the x-axis.
		 * @param[in] y This is how far we go along the y-axis.
		 */
		void SetComponents(double x, double y)
		{
			this->x = x;
			this->y = y;
		}

		/**
		 * Get the x and y components of this vector.  This method is provided merely
		 * for convenience and consistency as the said components are made public in this class.
		 *
		 * @param[out] x This is how far we go along the x-axis.
		 * @param[out] y This is how far we go along the y-axis.
		 */
		void GetComponents(double& x, double& y) const
		{
			x = this->x;
			y = this->y;
		}

		/**
		 * Set this vector to be the point in 2D space found using the given polar coordinates.
		 * 
		 * @param[in] angle This is the angle at which the point can be found, starting at the +X axis, positive values going counter-clock-wise about the origin.
		 * @param[in] radius This is the distance from the origin to the found point.
		 */
		void SetFromPolarCoords(double angle, double radius);

		/**
		 * Get the polar coordinates of this vector interpreted as a point in 2D space.
		 * 
		 * @param[out] angle This is assigned the angle at which this point can be found, starting at the +X axis, positive values going counter-clock-wise about the origin.
		 * @param[out] radius This is assigned the distance from the origin to this point.
		 */
		void GetToPolarCoords(double& angle, double& radius) const;

		/**
		 * Return the euclidean distance from tip to tail of this vector using the Pythagorean Theorem.
		 */
		double Length() const
		{
			return ::sqrt(this->Dot(*this));
		}

		/**
		 * Return the dot-product (or inner-product) of this vector and the given vector.
		 */
		double Dot(const Vector2& vector) const
		{
			return(
				this->x * vector.x +
				this->y * vector.y
			);
		}

		/**
		 * Return the cross-product of this vector and the given vector.  This is not
		 * to be confused with the outer product, because we don't return a blade of
		 * higher grade here; we return a scalar (which is a blade of lower grade.)
		 *
		 * @param[in] vectorA This first vector to be taken in the cross product.
		 * @param[in] vectorB The second vector to be taken in the cross product.
		 * @return The determinant of a 2x2 matrix having first column vectorA and second column vectorB is returned.
		 */
		double Cross(const Vector2& vectorA, const Vector2& vectorB)
		{
			return vectorA.x * vectorB.y - vectorA.y * vectorB.x;
		}

		/**
		 * Return a vector in the same direction as this, but of unit-length.
		 */
		Vector2 Normalized() const;

		/**
		 * Rescale this vector to be of unit-length.  Of course, the direction is unchanged.
		 *
		 * @param[out] length This is an optional parameter that, if given, will contain the length of the vector prior to normalization.
		 * @return True is returned on success; false, otherwise.  Failure can occur if the vector is zero or numerically too short in length.
		 */
		bool Normalize(double* length = nullptr);

		/**
		 * Write this vector to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this vector from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		double x, y;
	};

	/**
	 * Calculate and return the sum of the two given vectors.
	 */
	inline Vector2 operator+(const Vector2& vectorA, const Vector2& vectorB)
	{
		return Vector2(
			vectorA.x + vectorB.x,
			vectorA.y + vectorB.y
		);
	}

	/**
	 * Calculate and return the difference between the two given vectors.
	 */
	inline Vector2 operator-(const Vector2& vectorA, const Vector2& vectorB)
	{
		return Vector2(
			vectorA.x - vectorB.x,
			vectorA.y - vectorB.y
		);
	}

	inline Vector2 operator*(const Vector2& vector, double scalar)
	{
		return Vector2(
			vector.x * scalar,
			vector.y * scalar
		);
	}

	inline Vector2 operator*(double scalar, const Vector2& vector)
	{
		return Vector2(
			vector.x * scalar,
			vector.y * scalar
		);
	}

	inline Vector2 operator/(const Vector2& vector, double scalar)
	{
		return Vector2(
			vector.x / scalar,
			vector.y / scalar
		);
	}

	/**
	 * This is a component-wise multiply of the given vectors, not to be mistaken
	 * with something like the outer product, or the geometric product.
	 */
	inline Vector2 operator*(const Vector2& vectorA, const Vector2& vectorB)
	{
		return Vector2(
			vectorA.x * vectorB.x,
			vectorA.y * vectorB.y
		);
	}
}