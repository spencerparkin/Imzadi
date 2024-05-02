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
		SphereShape(bool temporary);
		virtual ~SphereShape();

		/**
		 * See Shape::GetShapeTypeID.
		 */
		virtual TypeID GetShapeTypeID() const override;

		/**
		 * See Shape::RecalculateCache.
		 */
		virtual void RecalculateCache() const override;

		/**
		 * Tell the caller if this sphere is valid.  The radius must be
		 * non-zero and non-negative.
		 *
		 * @return True is returened if valid; false, otherwise.
		 */
		virtual bool IsValid() const override;

		/**
		 * Calculate and return the volume of this sphere.
		 */
		virtual double CalcSize() const override;

		/**
		 * Tell the caller if the given point is contained within this sphere or on its surface.
		 */
		virtual bool ContainsPoint(const Vector3& point) const override;

		/**
		 * Allocate and return a new SphereShape class instance.
		 */
		static SphereShape* Create();

		/**
		 * Set the center location of the sphere in object space.
		 * 
		 * @param[in] center The sphere will be all points within at most the stored radius of this point.
		 */
		void SetCenter(const Vector3& center) { this->center = center; }

		/**
		 * Get the center location of the sphere in object space.
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
		double GetRadius() const { return this->radius; }

		/**
		 * Debug-draw this sphere shape in world-space.
		 */
		virtual void DebugRender(DebugRenderResult* renderResult) const override;

		/**
		 * Perform a ray-cast against this sphere.
		 * 
		 * @param[in] ray This is the ray to cast against this sphere.
		 * @param[out] alpha This is the distance from ray origin to the hit sphere point, if any.
		 * @param[out] unitSurfaceNormal This will be the surface normal of the sphere at the point of ray impact, if any.
		 * @return True is returned if the given ray hits this sphere; false, otherwise.
		 */
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const override;

	private:
		Vector3 center;
		double radius;
	};
}