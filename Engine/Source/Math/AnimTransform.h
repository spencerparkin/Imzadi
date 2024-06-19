#pragma once

#include "Quaternion.h"
#include "Vector3.h"

namespace Imzadi
{
	class Transform;

	/**
	 * These are transforms toted-about in a decomposed form that lend themselves
	 * more easily to interpolation at the cost of being a bit harder to concatinate.
	 * They're almost rigid-body transforms if it weren't for the possability of having
	 * non-uniform scale.  They have three parts: scale, rotation, and translation,
	 * which are applied in that order.
	 * 
	 * As far as the @ref Transform class goes, every AnimTransform is such an instance,
	 * but not every such instance is an AnimTransform.
	 */
	class IMZADI_API AnimTransform
	{
	public:
		AnimTransform();
		AnimTransform(const AnimTransform& transform);
		AnimTransform(const Vector3& scale, const Quaternion& rotation, const Vector3& translation);
		virtual ~AnimTransform();

		void operator=(const AnimTransform& transform);

		/**
		 * Set this instance to the identity transformation.  This is the default for any newly created instance.
		 */
		void SetIdentity();

		/**
		 * Set this transform using the given transform, which can have shear.
		 * If it does have shear, then we fail here.  Note that the conversion
		 * here is somewhat arbitrary as more than one answer is possible.
		 * 
		 * @param[in] transform This is the transform to convert, leaving it untouched, of course.
		 * @return True is returned on success; false, otherwise.
		 */
		bool SetFromTransform(const Transform& transform);

		/**
		 * Get this transform into the given transform.
		 * 
		 * @param[out] transform This transform receives the conversion of this transform.
		 * @return True is returned on success; false, otherwise.
		 */
		bool GetToTransform(Transform& transform) const;

		/**
		 * Apply this transformation to the given point and return the result.
		 */
		Vector3 TransformPoint(const Vector3& point) const;

		/**
		 * Apply this transformation to the given vector and return the result.
		 * The difference between this and @ref TransformPoint is that here we
		 * do not apply the translational component of the transform.
		 */
		Vector3 TransformVector(const Vector3& vector) const;

		/**
		 * Interpolate between the two given transforms by the given amount, and then
		 * store the result in this transform.
		 * 
		 * @param[in] transformA This is the result when alpha is zero.
		 * @param[in] transformB This is the result when alpha is one.
		 * @param[in] alpha This is typically a value in [0,1], but this need not be the case.
		 */
		void SetAsInterpolationOf(const AnimTransform& transformA, const AnimTransform& transformB, double alpha);

		/**
		 * Write this transform to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this transform from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		Vector3 scale;
		Quaternion rotation;
		Vector3 translation;
	};

	/**
	 * Concatinate the two given transforms and return the result.  This is a non-commutative operation.
	 *
	 * @param[in] transformA The first transform taken in the function composition.
	 * @param[in] transformB The second transform taken in the function composition.
	 * @return The transform composed by first evaluating transformB, then sending its result to transformA.
	 */
	IMZADI_API AnimTransform operator*(const AnimTransform& transformA, const AnimTransform& transformB);
}