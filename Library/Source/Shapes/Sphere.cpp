#include "Sphere.h"
#include "Math/AxisAlignedBoundingBox.h"

using namespace Collision;

SphereShape::SphereShape()
{
	this->radius = 1.0;
}

/*virtual*/ SphereShape::~SphereShape()
{
}

/*static*/ SphereShape* SphereShape::Create()
{
	return new SphereShape();
}

/*virtual*/ Shape::TypeID SphereShape::GetShapeTypeID() const
{
	return TypeID::SPHERE;
}

/*virtual*/ void SphereShape::CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	Vector3 delta(this->radius, this->radius, this->radius);
	Vector3 worldCenter = this->objectToWorld.TransformPoint(this->center);

	boundingBox.minCorner = worldCenter + delta;
	boundingBox.maxCorner = worldCenter - delta;
}

/*virtual*/ bool SphereShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (::isnan(this->radius) || ::isinf(this->radius))
		return false;

	if (!this->center.IsValid())
		return false;

	if (this->radius <= 0.0)
		return false;

	return true;
}

/*virtual*/ double SphereShape::CalcSize() const
{
	return (4.0 / 3.0) * M_PI * this->radius * this->radius * this->radius;
}

/*virtual*/ void SphereShape::DebugRender(DebugRenderResult* renderResult) const
{
}