#pragma once

#include "Defines.h"
#include <istream>
#include <ostream>

namespace Collision
{
	class Vector3;
	class Quaternion;

	/**
	 * Instances of this class are 3x3 matrices with real elements and form a group under addition,
	 * but not multiplication.  All non-singular (invertable) matrices of this sort do form a group
	 * under multiplication.
	 */
	class COLLISION_LIB_API Matrix3x3
	{
	public:
		Matrix3x3();
		Matrix3x3(const Matrix3x3& matrix);
		virtual ~Matrix3x3();

		/**
		 * Set this matrix as a copy of the given matrix.
		 */
		void operator=(const Matrix3x3& matrix);

		/**
		 * Accumulate the given matrix into this matrix.
		 */
		void operator+=(const Matrix3x3& matrix);

		/**
		 * Subtract the given matrix from this matrix.
		 */
		void operator-=(const Matrix3x3& matrix);

		/**
		 * Scale this matrix by the given scalar.
		 */
		void operator*=(double scalar);

		/**
		 * Tell the caller if Inf or NaN can be found in the matrix.
		 * No other notion of validity is checked here.  If we want to
		 * check that your matrix is non-singular, check the determinant.
		 * If you want to check that it's orthogonal, compare the row
		 * or column vectors.  If you want to check that you have a
		 * right-handed system, check the sign of the determinant.
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
		 * @param[out] zAxis This vector will contain the third row.
		 */
		void GetRowVectors(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;

		/**
		 * Assign the rows of this matrix from top to bottom as the given vectors in
		 * the order given.
		 * 
		 * @param[in] xAxis This vector is assigned to the first row.
		 * @param[in] yAxis This vector is assigned to the second row.
		 * @param[in] zAxis This vector is assigned to the third row.
		 */
		void SetRowVectors(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

		/**
		 * Return the i^{th} row-vector of this matrix.
		 * 
		 * @param[in] i This is an integer in [0,2] specifying the desired row.
		 */
		Vector3 GetRowVector(int i) const;

		/**
		 * Set the i^{th} row-vector of this matrix.
		 * 
		 * @param[in] i This is an integer in [0,2] specifying the desired row.
		 * @return The ith row is returned as a vector, left to right.
		 */
		void SetRowVector(int i, const Vector3& vector);

		/**
		 * Return the columns of this matrix from left to right in the given vectors in
		 * the order given.
		 * 
		 * @param[out] xAxis This vector will contain the first column.
		 * @param[out] yAxis This vector will contain the second column.
		 * @param[out] zAxis This vector will contain the third column.
		 */
		void GetColumnVectors(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;

		/**
		 * Assign the columns of this matrix from top to bottom as the given vectors in
		 * the order given.
		 * 
		 * @param[in] xAxis This vector is assigned to the first column.
		 * @param[in] yAxis This vector is assigned to the second column.
		 * @param[in] zAxis This vector is assigned to the third column.
		 */
		void SetColumnVectors(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

		/**
		 * Return the ith column-vector of this matrix.
		 *
		 * @param[in] i This is an integer in [0,2] specifying the desired column.
		 */
		Vector3 GetColumnVector(int i) const;

		/**
		 * Set the i^{th} column-vector of this matrix.
		 *
		 * @param[in] i This is an integer in [0,2] specifying the desired column.
		 * @return The i^{th} column is returned as a vector, top to bottom.
		 */
		void SetColumnVector(int i, const Vector3& vector);

		/**
		 * Formulate this matrix as a rotation matrix that performs the rotation
		 * determined by the given unit-length axis and angle pair.
		 * 
		 * @param[in] unitAxis This is the axis about which the rotation is performed by this matrix.  If not unit-length, the result is left undefined.
		 * @param[in] angle This is the angle, in radians, determining the amount of rotation performed.
		 */
		void SetFromAxisAngle(const Vector3& unitAxis, double angle);

		/**
		 * Extract from this matrix the unit-length axis and angle describing
		 * the rotation performed by this matrix.  If this matrix is not a rotation
		 * matrix, then the result is left undefined.  Note that any non-singular
		 * matrix that is orthonormalized becomes a rotation/orientation matrix,
		 * provided its determinant is positive.
		 * 
		 * @param[out] unitAxis This is the axis about which the rotation is formed by this matrix.  It will be of unit-length.  It is also an eigen-vector of the matrix.
		 * @param[out] angle This is the angle determining the amount of rotation performed.  It will be in radians.
		 */
		void GetToAxisAngle(Vector3& unitAxis, double& angle) const;

		/**
		 * Formulate this matrix as a rotation matrix performing the same
		 * rotation as that performed by the given quaternion.
		 * 
		 * @param[in] unitQuat This is the quaternion to use.  It is assumed to be of unit-magnitude.  If this is not the case, then the result is left undefined.
		 */
		void SetFromQuat(const Quaternion& unitQuat);

		/**
		 * Extract from this matrix the unit-magnitude quaternion representing
		 * the same rotation as that represented by this matrix.  If this matrix
		 * is not a rotation matrix, then the result is left undefined.
		 * 
		 * @param[out] unitQuat This is the returned quaternion.  It will be of unit-magnitude.
		 */
		void GetToQuat(Quaternion& unitQuat) const;

		/**
		 * TODO: Document this.
		 */
		void SetOuterProduct(const Vector3& vectorA, const Vector3& vectorB);

		/**
		 * TODO: Document this.
		 */
		void SetForCrossProduct(const Vector3& vector);

		/**
		 * Set this matrix to a uniform scale matrix.
		 * 
		 * @param[in] scale This is the uniform scale factor.
		 */
		void SetUniformScale(double scale);

		/**
		 * Set this matrix to a non-uniform scale matrix.
		 * 
		 * @param[in] scale This is a vector holding the scale factors along the various axes.
		 */
		void SetNonUniformScale(const Vector3& scale);

		/**
		 * Calculate and return the orthonormalization of this matrix.
		 * The result is left undefined if this matrix is singular.
		 * Occational orthonormalization is sometimes needed in order
		 * to eliminate accumulated round-off error due to, for example,
		 * continually multiplying a state matrix by a rotation matrix.
		 * Mathematically, you'd stay in the group of rotations, but in
		 * practice, error would build up, and occationally you'd need to
		 * re-orthonormalize your state matrix.
		 * 
		 * Note that this method does not change the sign of the determinant of the matrix.
		 * 
		 * @param[in] anchorAxis This is one of (not an Or-ing of) the COLL_SYS_AXIS_FLAG_* defines, and indicates the anchor-axis used for the orthonormalization process.
		 * @return An orthonormal matrix is returned as the result of the Gram-Schmit process performed on this matrix.
		 */
		Matrix3x3 Orthonormalized(uint32_t anchorAxis) const;

		/**
		 * Calculate and return the inverse of this matrix.
		 * The result is left undefined if this matrix is singular.
		 * See the Invert function if your matrix may be singular.
		 * 
		 * @return The multiplicative inverse of this matrix is returned.
		 */
		Matrix3x3 Inverted() const;

		/**
		 * Set this matrix as the multiplicative inverse of the given matrix.
		 * 
		 * @param matrix This is the matrix to be inverted.
		 * @return True is returned if the given matrix is non-singular; false, otherwise.
		 */
		bool Invert(const Matrix3x3& matrix);

		/**
		 * Calculate and return the transpose of this matrix.
		 * Note that the inverse of an orthonormal matrix is its transpose.
		 * Also, the inverse-transpose is sometimes needed to transform vertex
		 * normals in a way different than vertex points.
		 */
		Matrix3x3 Transposed() const;

		/**
		 * Set this matrix as the transpose of the given matrix.
		 * 
		 * @param matrix This is the matrix to be transposed.
		 */
		void Transpose(const Matrix3x3& matrix);

		/**
		 * Calculate and return the determinant of this matrix.
		 * This is zero if and only if the matrix is singular.
		 * Geometrically, this can be interpreted as the signed volume of the
		 * parallel-piped formed by taking the row (or column) vectors
		 * of the matrix and placing them tail-to-tail-to-tail.
		 * A positive value indicates a right-handed system; left-handed, otherwise.
		 * 
		 * @return The determinant of this matrix is returned.
		 */
		double Determinant() const;

		/**
		 * Factor this matrix into a rotation, scale and shear matrix,
		 * multiplied in that order.
		 * 
		 * @param[out] rotation This will be the rotation matrix factor.  It is orthonormal with determinant +1.
		 * @param[out] scale This will be the scale matrix factor, not necessarily uniform.
		 * @param[out] shear This will be the shear matrix factor.
		 * @return True is returned on success; false, otherwise.  Failure occurs if and only if the matrix is singular.
		 */
		bool Factor(Matrix3x3& rotation, Matrix3x3& scale, Matrix3x3& shear) const;

		/**
		 * Calculate this matrix and an interpolation between the two given orientation matrices.
		 * If the two given matrices are not orthonormal with determinant +1, then our result here
		 * is left undefined.  We interpolate by rotating the first matrix into the other.  Since
		 * rotation matrices form a group, we can divide one by the other and get a rotation matrix,
		 * which can be thought of as an axis/angle pair.  The given alpha value is used to scale
		 * this rotation angle.
		 * 
		 * @param[in] orientationA This is the matrix we get with alpha at zero.
		 * @param[in] orientationB This is the matrix we get with alpha at one.
		 * @param[in] alpha This is the interpolation value ranging from zero to one.
		 */
		void InterpolateOrientations(const Matrix3x3& orientationA, const Matrix3x3& orientationB, double alpha);

		/**
		 * Write this matrix to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this matrix from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		double ele[3][3];
	};

	/**
	 * Calculate and return the sum of the two given 3x3 matrices.  This is a commutative operation.
	 * 
	 * @param[in] matrixA The first matrix taken in the sum.
	 * @param[in] matrixB The second matrix taken in the sum.
	 * @return The sum of matrixA and matrixB is returned.
	 */
	COLLISION_LIB_API Matrix3x3 operator+(const Matrix3x3& matrixA, const Matrix3x3& matrixB);

	/**
	 * Calculate and return the difference of the two given 3x3 matrices.  This is a non-commutative operation.
	 * 
	 * @param[in] matrixA The first matrix taken in the difference.
	 * @param[in] matrixB The second matrix taken in the difference.
	 * @return The second matrix taken away from the first is returned.
	 */
	COLLISION_LIB_API Matrix3x3 operator-(const Matrix3x3& matrixA, const Matrix3x3& matrixB);

	/**
	 * Calculate and return the product of the two given 3x3 matrices.  This is a non-commutative operation.
	 * 
	 * @param[in] matrixA The first matrix taken in the product.
	 * @param[in] matrixB The second matrix taken in the product.
	 * @return The product of matrixA and matrixB is returned, in that order.
	 */
	COLLISION_LIB_API Matrix3x3 operator*(const Matrix3x3& matrixA, const Matrix3x3& matrixB);

	/**
	 * Calculate and return the quotient of the two given 3x3 matrices.  This is a non-commutative operation,
	 * and is left undefined in the case that matrixB is singular (non-invertable.)
	 * 
	 * @param[in] matrixA The first matrix taken in the quotient (the dividend.)
	 * @param[in] matrixB The second matrix taken in the quotient (the divisor.)
	 * @return The quotient of matrixA and matrixB is returned, in that order.
	 */
	COLLISION_LIB_API Matrix3x3 operator/(const Matrix3x3& matrixA, const Matrix3x3& matrixB);

	/**
	 * Calculate and return the given 3x3 matrix scaled by the given scalar.  This is a commutative operation.
	 * 
	 * @param[in] matrix This is the matrix to be scaled.
	 * @param[in] scalar This is the amount by which the matrix is scaled.
	 * @return The product of the matrix and the scalar is returned.
	 */
	COLLISION_LIB_API Matrix3x3 operator*(const Matrix3x3& matrix, double scalar);

	/**
	 * See the documentation for the other variation of this function.
	 */
	COLLISION_LIB_API Matrix3x3 operator*(double scalar, const Matrix3x3& matrix);

	/**
	 * Treating the given vector as a 3x1 column vector, calculate and return
	 * the prodcut of the given 3x3 matrix and the given vector, in that order.
	 * 
	 * @param[in] matrix This is the matrix taken in the product.
	 * @param[in] vector This is the vector taken in the product.
	 * @return A 3x1 column vector is returned.
	 */
	COLLISION_LIB_API Vector3 operator*(const Matrix3x3& matrix, const Vector3& vector);

	/**
	 * Treating the given vector as a 1x3 row vector, calculate and return
	 * the product of the given vector and the given 3x3 matrix, in that order.
	 * 
	 * @param[in] vector This is the vector taken in the product.
	 * @param[in] matrix This is the matrix taken in the product.
	 * @return A 1x3 row vector is returned.
	 */
	COLLISION_LIB_API Vector3 operator*(const Vector3& vector, const Matrix3x3& matrix);
}