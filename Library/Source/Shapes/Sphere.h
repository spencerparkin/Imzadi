#pragma once

#include "Shape.h"
#include "Math/Vector3.h"

namespace Collision
{
	/**
	 * This is a collision shape defined as all points within a given radius of a center point.
	 */
	class COLLISION_LIB_API SphereShape : public Shape
	{
	public:
		SphereShape();
		virtual ~SphereShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;

		/**
		 * Set the center location of the sphere.
		 * 
		 * @param[in] center The sphere will be all points within at most the stored radius of this point.
		 */
		void SetCenter(const Vector3& center) { this->center = center; }

		/**
		 * Get the center location of the sphere.
		 */
		const Vector3& GetCenter() const { return this->center; }

		/**
		 * Set the radius of the sphere.
		 * 
		 * @param radius This will be the radius of the sphere.  It must be non-zero and non-negative.
		 */
		void SetRadius(double radius) { this->radius = radius; }

		/**
		 * Get the radius of the sphere.
		 */
		double GetRadias() const { return this->radius; }

	private:
		Vector3 center;
		double radius;
	};
}