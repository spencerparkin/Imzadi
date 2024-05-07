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

		/**
		 * See Shape::GetShapeTypeID.
		 */
		virtual TypeID GetShapeTypeID() const override;

		/**
		 * Return what we do in GetShapeTypeID().
		 */
		static TypeID StaticTypeID();

		/**
		 * See Shape::RecalculateCache.
		 */
		virtual void RecalculateCache() const override;

		/**
		 * Tell the caller if this capsule is valid.
		 * 
		 * @return True is returened if valid; false, otherwise.
		 */
		virtual bool IsValid() const override;

		/**
		 * Calculate and return the volume of this capsule.
		 */
		virtual double CalcSize() const override;

		/**
		 * Tell the caller if the given point is contained within this capsule or on its surface.
		 */
		virtual bool ContainsPoint(const Vector3& point) const override;

		/**
		 * Render this capsule as wire-frame in the given result.
		 */
		virtual void DebugRender(DebugRenderResult* renderResult) const override;

		/**
		 * Perform a ray-cast against this capsule.
		 * 
		 * @param[in] ray This is the ray to use in the ray-cast.
		 * @param[out] alpha This is the distance from the ray origin along the ray-direction to the point where the capsule is hit, if any.
		 * @param[out] unitSurfaceNormal This is the surface normal of the capsule at the ray hit-point, if any.
		 */
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const override;

		/**
		 * Write this capsule to given stream in binary form.
		 */
		virtual bool Dump(std::ostream& stream) const override;

		/**
		 * Read this capsule from the given stream in binary form.
		 */
		virtual bool Restore(std::istream& stream) override;

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
		 * Return the line segment that serves as the spine of this capsule.
		 */
		const LineSegment& GetSpine() const { return this->lineSegment; }

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