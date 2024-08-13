#pragma once

#include "Collision/Shape.h"
#include "Math/Vector3.h"

namespace Imzadi {
namespace Collision {

/**
 * This is a collision shape defined as all points within a given radius of a center point.
 */
class IMZADI_API SphereShape : public Shape
{
	friend class SphereShapeCache;

public:
	SphereShape();
	virtual ~SphereShape();

	/**
	 * See Shape::GetShapeTypeID.
	 */
	virtual TypeID GetShapeTypeID() const override;

	/**
	 * Return what we do in GetShapeTypeID().
	 */
	static TypeID StaticTypeID();

	/**
	 * Tell the caller if this sphere is valid.  The radius must be
	 * non-zero and non-negative.
	 *
	 * @return True is returened if valid; false, otherwise.
	 */
	virtual bool IsValid() const override;

	/**
	 * Allocate and return a Sphere shape that is a copy of this sphere.
	 */
	virtual Shape* Clone() const override;

	/**
	 * Make this sphere the same as the given sphere.
	 */
	virtual bool Copy(const Shape* shape) override;

	/**
	 * Calculate and return the volume of this sphere.
	 */
	virtual double CalcSize() const override;

	/**
	 * Tell the caller if the given point is contained within this sphere or on its surface.
	 */
	virtual bool ContainsPoint(const Vector3& point) const override;

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

	/**
	 * Write this sphere to given stream in binary form.
	 */
	virtual bool Dump(std::ostream& stream) const override;

	/**
	 * Read this sphere from the given stream in binary form.
	 */
	virtual bool Restore(std::istream& stream) override;

protected:

	/**
	 * Allocate and return the shape cache (SphereShapeCache) used by this class.
	 */
	virtual ShapeCache* CreateCache() const override;

private:
	Vector3 center;
	double radius;
};

/**
 * This class knows how to regenerate cache for a SphereShape class.
 */
class SphereShapeCache : public ShapeCache
{
public:
	SphereShapeCache();
	virtual ~SphereShapeCache();

	/**
	 * Update the given sphere's bounding box.
	 */
	virtual void Update(const Shape* shape) override;
};

} // namespace Collision
} // namespace Imzadi