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
		BoxShape();
		virtual ~BoxShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void RecalculateCache() const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;
		virtual void DebugRender(DebugRenderResult* renderResult) const override;

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

	private:
		Vector3 extents;
	};
}