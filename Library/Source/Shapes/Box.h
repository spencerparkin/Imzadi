#pragma once

#include "Shape.h"
#include "Math/Vector3.h"

namespace Collision
{
	/**
	 * This collision shape is simply a box with a given width, height and depth.
	 * In object-space, the box is centered at origin having a width in the x-dimension,
	 * height in the y-dimension, and depth in the z-dimension.
	 */
	class COLLISION_LIB_API BoxShape : public Shape
	{
	public:
		BoxShape(bool temporary);
		virtual ~BoxShape();

		/**
		 * See Shape::GetShapeTypeID.
		 */
		virtual TypeID GetShapeTypeID() const override;

		/**
		 * See Shape::RecalculateCache.
		 */
		virtual void RecalculateCache() const override;

		/**
		 * Tell the caller if this box is valid.  A valid box
		 * will have non-zero, positive extents.
		 * 
		 * @return True is returned if valid; false, otherwise.
		 */
		virtual bool IsValid() const override;

		/**
		 * Calculate and return the volume of this box.
		 */
		virtual double CalcSize() const override;

		/**
		 * Render this box as wire-frame in the given result.
		 */
		virtual void DebugRender(DebugRenderResult* renderResult) const override;

		/**
		 * Perform a ray-cast against this box.
		 * 
		 * @param[in] ray This is the ray to use in the ray-cast.
		 * @param[out] alpha This is the distance from the ray origin along the ray-direction to the point where this box is hit, if any.
		 * @param[out] unitSurfaceNormal This is the surface normal of the box at the ray hit-point, if any.
		 */
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const override;

		/**
		 * Allocate and return a new BoxShape class instance.
		 */
		static BoxShape* Create();

		/**
		 * Set this box's half-width, half-height, and half-depth.  You can think of the
		 * extent vector as pointing from the center of the box to one of its corners.
		 * All components of the given vector should be non-zero and non-negative.
		 * 
		 * @param[in] extents Here, the x-component is the half-size of the box in the x-dimension while in object space.  The same goes for the other components.
		 */
		void SetExtents(const Vector3& extents) { this->extents = extents; }

		/**
		 * Get this box's half-width, half-height, and half-depth.
		 * 
		 * @return See the description for SetExtents.
		 */
		const Vector3& GetExtents() const { return this->extents; }

		/**
		 * Calculate and return the corner points of this box.
		 * 
		 * @param[out] cornerPointArray This is populated with this box's corners.
		 * @param[in] worldSpace If true, the world-space corners are calculated; the object-space corners, otherwise.
		 */
		void GetCornerPointArray(std::vector<Vector3>& cornerPointArray, bool worldSpace) const;

	private:
		Vector3 extents;
	};
}