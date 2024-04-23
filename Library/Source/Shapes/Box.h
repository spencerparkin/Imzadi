#pragma once

#include "Shape.h"
#include "Math/Vector3.h"
#include "Math/Transform.h"

namespace Collision
{
	/**
	 * This collision shape is a box that can be oriented and positioned in anyway.
	 * (Unlike an AABB, it need not be axis-aligned.)  In object-space, the box is
	 * centered at origin having a width in the x-dimension, height in the y-dimension,
	 * and depth in the z-dimension.
	 */
	class COLLISION_LIB_API BoxShape : public Shape
	{
	public:
		BoxShape();
		virtual ~BoxShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;

		/**
		 * Set this box's width, height, and depth.
		 * 
		 * @param[in] extents Here, the x-component is the size of the box in the x-dimension while in object space.  The same goes for the other components.
		 */
		void SetExtents(const Vector3& extents) { this->extents = extents; }

		/**
		 * Get this box's width, height, and depth.
		 * 
		 * @return The width, height, and depth of the box, in object space, are returned as the components of the returned vector.
		 */
		const Vector3& GetExtents() const { return this->extents; }

		/**
		 * Set this box's object-space to world-space transform.
		 */
		void SetTransform(const Transform& objectToWorld) { this->objectToWorld = objectToWorld; }

		/**
		 * Get this box's object-space to world-space transform.
		 */
		const Transform& GetTransform() const { return this->objectToWorld; }

	private:
		Vector3 extents;
		Transform objectToWorld;
	};
}