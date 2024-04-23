#pragma once

#include "Shape.h"
#include "Math/Vector3.h"

namespace Collision
{
	/**
	 * This is a collision shape defined as all points withina given radius of a line-segment.
	 * You can think of it like a cylinder with hemi-spheres on both ends.
	 */
	class COLLISION_LIB_API CapsuleShape : public Shape
	{
	public:
		CapsuleShape();
		virtual ~CapsuleShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;

		/**
		 * Set one of the two vertices of this capsule.
		 * 
		 * @param[in] i If even, the first vertex is set; the other, if odd.
		 * @param[in] point This point is assigned to vertex i.
		 */
		void SetVertex(int i, const Vector3& point);

		/**
		 * Get one of the two vertices of this capsule.
		 * 
		 * @param[in] i If even, the first vertex is returned; the other, if odd.
		 * @return The point assigned to vertex i is returned.
		 */
		const Vector3& GetVertex(int i) const;

		/**
		 * Set this capsule's radius.
		 */
		void SetRadius(double radius) { this->radius = radius; }

		/**
		 * Get this capsule's radius.
		 */
		double GetRadius() const { return this->radius; }

	private:
		Vector3 vertex[2];
		double radius;
	};
}