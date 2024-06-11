#pragma once

#include "Defines.h"
#include <vector>

namespace Imzadi
{
	class AxisAlignedBoundingBox;
	class Matrix4x4;
	class Plane;
	class Vector3;
	class Transform;

	/**
	 * Frustums can be thought of as volumes in the shape of a pyramid, but the
	 * tip is chopped off, so you get a 6-sided shape similar to a box, but the
	 * corners don't meet at right angle.  These are most often used to represent
	 * a camera's field of vision, and projection matrices are based on the idea
	 * of warping this space into a box that does have right angles.  This warping
	 * of the space is what gives us the fore-shortening effect; which is the effect
	 * that things further away appear smaller, and things closer, bigger.
	 * 
	 * The frustums represented by this class can very in shape, but not position
	 * and orientation.  Specifically, these frustums always have the tip of their
	 * pyramid at the origin, and the base goes toward the -Z axis.  In other words,
	 * these are always camera-space frustums.  If you want to check to see if a
	 * world space object intersects a frustum, then you need to transform that
	 * object into camera space first.
	 */
	class IMZADI_API Frustum
	{
	public:
		Frustum();
		Frustum(const Frustum& frustum);
		virtual ~Frustum();

		void operator=(const Frustum& frustum);

		/**
		 * Return true if and only if the given sphere non-trivially overlaps this frustum.
		 * That is, if the overlapping area is non-zero.
		 * 
		 * @param[in] sphere This should be a sphere in camera-space.
		 */
		bool IntersectedBySphere(const Vector3& center, double radius) const;

		/**
		 * Calculate and return the 6 planes that form the sides of this frustum.
		 */
		void GetPlanes(std::vector<Plane>& planeArray) const;

		/**
		 * Calculate and return the projection matrix that can be used for
		 * camera space to projection space transformations, the said
		 * camera space being represented by the area inside this frustum.
		 * Camera space is thought of as the eye at origin looking down the
		 * -Z axis with +Y axis up, and +X axis right.
		 * 
		 * @param[out] matrix This is the returned projection matrix.
		 */
		void GetToProjectionMatrix(Matrix4x4& matrix) const;

		/**
		 * TODO: Write this.
		 */
		bool SetFromProjectionMatrix(const Matrix4x4& matrix);

		/**
		 * Initialize this frustum using an aspect-ratio and a horizontal field of vision.
		 * 
		 * @param[in] aspectRatio This is the width to height ratio of the projection screen.
		 * @param[in] hfovi This is the angle of the horizontal field of vision in radians.
		 * @param[in] nearClip This is the distance along the -Z axis to the near clipping plane.  It is a positive value.
		 * @param[in] farClip This is the distance along the -Z axis to the far clipping plane.  It is a postivie value.
		 */
		void SetFromAspectRatio(double aspectRatio, double hfovi, double nearClip, double farClip);

	public:
		double hfovi;		///< This is the horizontal field of vision in radians.
		double vfovi;		///< This is the vertical field of vision in radians.
		double nearClip;	///< This is where the tip of the pryamid is chopped off.
		double farClip;		///< This is where the base of the pyrmaid is located.
	};
}