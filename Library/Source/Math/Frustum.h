#pragma once

#include "Defines.h"
#include <vector>

namespace Collision
{
	class AxisAlignedBoundingBox;
	class Matrix4x4;
	class Plane;

	/**
	 * Frustums can be thought of as volumes in the shape of a pyramid, but the
	 * tip is chopped off, so you get a 6-sided shape similar to a box, but the
	 * corners don't meet at right angle.  These are most often used to represent
	 * a camera's field of vision, and projection matrices are based on the idea
	 * of warping this space into a box that does have right angles.  This warping
	 * of the space is what gives us the fore-shortening effect, which is the effect
	 * that things further away appear smaller, and things closer, bigger.
	 * 
	 * The frustums represented by this class can very in shape, but not position
	 * and orientation.  Specifically, these frustums always have the tip of their
	 * pyramid at the origin, and the base goes toward the -Z axis.
	 */
	class COLLISION_LIB_API Frustum
	{
	public:
		Frustum();
		Frustum(const Frustum& frustum);
		virtual ~Frustum();

		void operator=(const Frustum& frustum);

		/**
		 * Return true if and only if the given box non-trivially overlaps this frustum.
		 * That is, if the overlapping area is non-zero.
		 */
		bool IntersectedBy(const AxisAlignedBoundingBox& box) const;

		/**
		 * Calculate and return the 6 planes that form the sides of this frustum.
		 */
		void GetPlanes(std::vector<Plane>& planeArray) const;

		/**
		 * Calculate and return the projection matrix that can be used for
		 * world to camera space transforms, the said camera space being
		 * the area inside this frustum.
		 * 
		 * @param[out] matrix This is the returned projection matrix.
		 */
		void ToProjectionMatrix(Matrix4x4& matrix) const;

		/**
		 * Calculate this frustum as that represented by the given
		 * world to camera space transform.  If the given matrix
		 * does not represent such a transform, then we leave our
		 * result here undefined.
		 * 
		 * @param[in] matrix This matrix is assumed to be a projection matrix similar to what's calculated in the ToProjectionMatrix method.
		 * @return True is returned on success; false, otherwise.
		 */
		bool FromProjectionMatrix(const Matrix4x4& matrix);

	public:
		double hfovi;		///< This is the horizontal field of vision in radians.
		double vfovi;		///< This is the vertical field of vision in radians.
		double near;		///< This is where the tip of the pryamid is chopped off.
		double far;			///< This is where the base of the pyrmaid is located.
	};
}