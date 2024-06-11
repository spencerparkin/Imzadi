#pragma once

#include "Vector3.h"
#include "Matrix3x3.h"

namespace Imzadi
{
	class Plane;
	class Ray;
	class LineSegment;
	class Matrix4x4;

	/**
	 * These are affine transformations and can be considered vector-valued functions of a vector variable.
	 * They can also be thought of as augmented 4x4 matrices.  The upper-left 3x3 matrix is stored as possibly
	 * an orientation (or something else), and the upper-right 3x1 matrix is stored as a translation.
	 * The bottom row is all zeros except for a one in the lower-right corner.  4x4 matrix algebra, however,
	 * should not be assumed here.  We're just a function that is defined in terms of a 3x3 matrix and a vector.
	 * 4x4 matrix algebra might aide in formulating some of the methods, but it is not a user-facing concept.
	 * We're not doing anything here with homogenous coordinates, for example.
	 */
	class IMZADI_API Transform
	{
	public:
		/**
		 * A default transform will be an identity transform.
		 */
		Transform();

		/**
		 * Initialize this transform as the rigid-body transform determined by
		 * the given orientation and translation.
		 * 
		 * @param[in] unitAxis This is the axis of rotation for the orientation.
		 * @param[in] angle This is the angle, in radians, of the rotation for the orientation.
		 * @param[in] translation This is the translation, applied after rotation.
		 */
		Transform(const Vector3& unitAxis, double angle, const Vector3& translation);

		/**
		 * Initialize this transform with the given parameters.
		 * 
		 * @param[in] matrix This will be the stored 3x3 matrix, sometimes used as an orientation if orthonormal.
		 * @param[in] translation This will be the stored translation.
		 */
		Transform(const Matrix3x3& matrix, const Vector3& translation);

		/**
		 * Initialize this tranform as a copy of the given transform.
		 * 
		 * @param[in] transform This is the transform to copy.
		 */
		Transform(const Transform& transform);

		/**
		 * The destructor doesn't do anything.
		 */
		virtual ~Transform();

		/**
		 * Here we just check for the presents of Inf or Nan in the matrix and translation.
		 * 
		 * @return True is returned if the matrix is valid; false, otherwise.
		 */
		bool IsValid() const;

		/**
		 * Set this transform to be the identity affine transform.
		 */
		void SetIdentity();

		/**
		 * Apply our matrix to the given point, then the translation, and then return the result.
		 * 
		 * @param[in] point This is the point to be transformed.
		 * @return The transformed point is returned.
		 */
		Vector3 TransformPoint(const Vector3& point) const;

		/**
		 * Apply our matrix to the given normal, but not the translation, and then return the result.
		 * Note that if there is any shear or non-uniform scale in the matrix, then it's the
		 * inverse-transpose that really needs to be applied to a normal.  We don't do that here.
		 * 
		 * @param[in] normal This is the normal to be transformed.
		 * @return The transformed normal is returned.
		 */
		Vector3 TransformNormal(const Vector3& normal) const;

		/**
		 * Transform the two points of the given line-segment and return them as a new line-segment.
		 * 
		 * @param[in] lineSegment This is the line-segment to transform.
		 * @return The transformed line-segment is returned.
		 */
		LineSegment TransformLineSegment(const LineSegment& lineSegment) const;

		/**
		 * Transform the center and normal of the given plane using TransformPoint and TransformNormal, respectively.
		 * 
		 * @param[in] plane This is the plane to be transformed.
		 * @return The transformed plane is returned.
		 */
		Plane TransformPlane(const Plane& plane) const;

		/**
		 * Transform the origin and direction of the given ray using TransformPoint and TransformNormal, respectively.
		 * 
		 * @param[in] ray This is the ray to be transformed.
		 * @return The transformed ray is returend.
		 */
		Ray TransformRay(const Ray& ray) const;

		/**
		 * Calculate and return the inverse of this transformation.  If there
		 * is no inverse, then the result is left undefined.
		 * 
		 * @return A transform is returned that "undoes" what this transform does.
		 */
		Transform Inverted() const;

		/**
		 * Set this transform to be the inverse of the given transform.
		 * The given transform is invertible if and only if its stored
		 * 3x3 matrix is non-singular.
		 * 
		 * @param[in] transform This is the transform to be inverted.
		 * @return True is returned on success; false, otherwise.
		 */
		bool Invert(const Transform& transform);

		/**
		 * Return this transform in the given matrix.
		 * 
		 * @param[out] matrix This matrix will get initialized to one performing the same transform as this that performed by this class.
		 */
		void GetToMatrix(Matrix4x4& matrix) const;

		/**
		 * Initialize this transform using the given matrix.
		 * 
		 * @param[in] matrix This matrix is used to initialize this transform.
		 * @return We fail here if the given matrix isn't an affine transform.
		 */
		bool SetFromMatrix(const Matrix4x4& matrix);

		/**
		 * Write this transform to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this transform from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		Matrix3x3 matrix;
		Vector3 translation;
	};

	/**
	 * Concatinate the two given transforms and return the result.  This is a non-commutative operation.
	 * 
	 * @param[in] transformA The first transform taken in the function composition.
	 * @param[in] transformB The second transform taken in the function composition.
	 * @return The transform composed by first evaluating transformB, then sending its result to transformA.
	 */
	IMZADI_API Transform operator*(const Transform& transformA, const Transform& transformB);
}