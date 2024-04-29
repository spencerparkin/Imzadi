#pragma once

#include "Shape.h"
#include "Math/LineSegment.h"

namespace Collision
{
	/**
	 * This is a collision shape defined as all points within a given radius of a line-segment.
	 * You can think of it like a cylinder with hemi-spheres on both ends.  The line-segment
	 * is stored in object space, and an object-to-world transform is used to realize the
	 * capsule in world space.
	 */
	class COLLISION_LIB_API CapsuleShape : public Shape
	{
	public:
		CapsuleShape(bool temporary);
		virtual ~CapsuleShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void RecalculateCache() const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;
		virtual bool ContainsPoint(const Vector3& point) const override;
		virtual void DebugRender(DebugRenderResult* renderResult) const override;
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const override;

		/**
		 * Allocate and return a new CapsuleShape class instance.
		 */
		static CapsuleShape* Create();

		/**
		 * Set one of the two object-space vertices of this capsule.
		 * 
		 * @param[in] i If even, the first vertex is set; the other, if odd.
		 * @param[in] point This point is assigned to vertex i.
		 */
		void SetVertex(int i, const Vector3& point);

		/**
		 * Get one of the two object-space vertices of this capsule.
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
		LineSegment lineSegment;
		double radius;
	};
}