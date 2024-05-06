#pragma once

#include "Defines.h"

namespace Collision
{
	class Vector2;

	/**
	 * Instance of this class are 2x2 matrices with real elements.  Though the
	 * library is primarly concerned with 3D space, 2-dimensional problems
	 * often arrise.
	 */
	class COLLISION_LIB_API Matrix2x2
	{
	public:
		Matrix2x2();
		Matrix2x2(const Matrix2x2& matrix);
		virtual ~Matrix2x2();

		/**
		 * Set this matrix as a copy of the given matrix.
		 */
		void operator=(const Matrix2x2& matrix);

		/**
		 * Accumulate the given matrix into this matrix.
		 */
		void operator+=(const Matrix2x2& matrix);

		/**
		 * Subtract the given matrix from this matrix.
		 */
		void operator-=(const Matrix2x2& matrix);

		/**
		 * Scale this matrix by the given scalar.
		 */
		void operator*=(double scalar);

		/**
		 * Tell the caller if Inf or NaN can be found in the matrix.
		 *
		 * @return True is returned if the matrix is valid; false, otherwise.
		 */
		bool IsValid() const;

		/**
		 * Set this matrix to the multiplicative identity.
		 */
		void SetIdentity();

		/**
		 * Return the rows of this matrix from top to bottom in the given vectors in
		 * the order given.
		 *
		 * @param[out] xAxis This vector will contain the first row.
		 * @param[out] yAxis This vector will contain the second row.
		 */
		void GetRowVectors(Vector2& xAxis, Vector2& yAxis) const;

		/**
		 * Assign the rows of this matrix from top to bottom as the given vectors in
		 * the order given.
		 *
		 * @param[in] xAxis This vector is assigned to the first row.
		 * @param[in] yAxis This vector is assigned to the second row.
		 */
		void SetRowVectors(const Vector2& xAxis, const Vector2& yAxis);

		/**
		 * Return the columns of this matrix from left to right in the given vectors in
		 * the order given.
		 *
		 * @param[out] xAxis This vector will contain the first column.
		 * @param[out] yAxis This vector will contain the second column.
		 */
		void GetColumnVectors(Vector2& xAxis, Vector2& yAxis) const;

		/**
		 * Assign the columns of this matrix from top to bottom as the given vectors in
		 * the order given.
		 *
		 * @param[in] xAxis This vector is assigned to the first column.
		 * @param[in] yAxis This vector is assigned to the second column.
		 */
		void SetColumnVectors(const Vector2& xAxis, const Vector2& yAxis);

		/**
		 * Calculate and return the inverse of this matrix.
		 * The result is left undefined if this matrix is singular.
		 * See the Invert function if your matrix may be singular.
		 *
		 * @return The multiplicative inverse of this matrix is returned.
		 */
		Matrix2x2 Inverted() const;

		/**
		 * Set this matrix as the multiplicative inverse of the given matrix.
		 *
		 * @param matrix This is the matrix to be inverted.
		 * @return True is returned if the given matrix is non-singular; false, otherwise.
		 */
		bool Invert(const Matrix2x2& matrix);

		/**
		 * Calculate and return the transpose of this matrix.
		 */
		Matrix2x2 Transposed() const;

		/**
		 * Set this matrix as the transpose of the given matrix.
		 *
		 * @param matrix This is the matrix to be transposed.
		 */
		void Transpose(const Matrix2x2& matrix);

		/**
		 * Calculate and return the determinant of this matrix.
		 * This is zero if and only if the matrix is singular.
		 * Geometrically, this can be interpreted as the signed area of the
		 * parallelagram formed by taking the row (or column) vectors
		 * of the matrix and placing them tail-to-tail.
		 *
		 * @return The determinant of this matrix is returned.
		 */
		double Determinant() const;

	public:
		double ele[2][2];
	};

	/**
	 * Calculate and return the sum of the two given 2x2 matrices.  This is a commutative operation.
	 *
	 * @param[in] matrixA The first matrix taken in the sum.
	 * @param[in] matrixB The second matrix taken in the sum.
	 * @return The sum of matrixA and matrixB is returned.
	 */
	COLLISION_LIB_API Matrix2x2 operator+(const Matrix2x2& matrixA, const Matrix2x2& matrixB);

	/**
	 * Calculate and return the difference of the two 2x2 given matrices.  This is a non-commutative operation.
	 *
	 * @param[in] matrixA The first matrix taken in the difference.
	 * @param[in] matrixB The second matrix taken in the difference.
	 * @return The second matrix taken away from the first is returned.
	 */
	COLLISION_LIB_API Matrix2x2 operator-(const Matrix2x2& matrixA, const Matrix2x2& matrixB);

	/**
	 * Calculate and return the product of the two given 2x2 matrices.  This is a non-commutative operation.
	 *
	 * @param[in] matrixA The first matrix taken in the product.
	 * @param[in] matrixB The second matrix taken in the product.
	 * @return The product of matrixA and matrixB is returned, in that order.
	 */
	COLLISION_LIB_API Matrix2x2 operator*(const Matrix2x2& matrixA, const Matrix2x2& matrixB);

	/**
	 * Calculate and return the quotient of the two given 2x2 matrices.  This is a non-commutative operation,
	 * and is left undefined in the case that matrixB is singular (non-invertable.)
	 *
	 * @param[in] matrixA The first matrix taken in the quotient (the dividend.)
	 * @param[in] matrixB The second matrix taken in the quotient (the divisor.)
	 * @return The quotient of matrixA and matrixB is returned, in that order.
	 */
	COLLISION_LIB_API Matrix2x2 operator/(const Matrix2x2& matrixA, const Matrix2x2& matrixB);

	/**
	 * Calculate and return the given 2x2 matrix scaled by the given scalar.  This is a commutative operation.
	 *
	 * @param[in] matrix This is the matrix to be scaled.
	 * @param[in] scalar This is the amount by which the matrix is scaled.
	 * @return The product of the matrix and the scalar is returned.
	 */
	COLLISION_LIB_API Matrix2x2 operator*(const Matrix2x2& matrix, double scalar);

	/**
	 * See the documentation for the other variation of this function.
	 */
	COLLISION_LIB_API Matrix2x2 operator*(double scalar, const Matrix2x2& matrix);

	/**
	 * Treating the given vector as a 2x1 column vector, calculate and return
	 * the prodcut of the given 2x2 matrix and the given vector, in that order.
	 *
	 * @param[in] matrix This is the matrix taken in the product.
	 * @param[in] vector This is the vector taken in the product.
	 * @return A 2x1 column vector is returned.
	 */
	COLLISION_LIB_API Vector2 operator*(const Matrix2x2& matrix, const Vector2& vector);

	/**
	 * Treating the given vector as a 1x2 row vector, calculate and return
	 * the product of the given vector and the 2x2 given matrix, in that order.
	 *
	 * @param[in] vector This is the vector taken in the product.
	 * @param[in] matrix This is the matrix taken in the product.
	 * @return A 1x2 row vector is returned.
	 */
	COLLISION_LIB_API Vector2 operator*(const Vector2& vector, const Matrix2x2& matrix);
}