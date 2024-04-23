#include "Capsule.h"
#include "Math/AxisAlignedBoundingBox.h"

using namespace Collision;

CapsuleShape::CapsuleShape()
{
	this->radius = 1.0;
}

/*virtual*/ CapsuleShape::~CapsuleShape()
{
}

/*virtual*/ Shape::TypeID CapsuleShape::GetShapeTypeID() const
{
	return TypeID::POLYGON;
}

/*virtual*/ void CapsuleShape::CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	boundingBox.minCorner = this->vertex[0];
	boundingBox.maxCorner = boundingBox.minCorner;

	Vector3 delta(this->radius, this->radius, this->radius);

	boundingBox.Expand(this->vertex[0] + delta);
	boundingBox.Expand(this->vertex[0] - delta);
	boundingBox.Expand(this->vertex[1] + delta);
	boundingBox.Expand(this->vertex[1] - delta);
}

/*virtual*/ bool CapsuleShape::IsValid() const
{
	if (::isnan(this->radius) || ::isinf(this->radius))
		return false;

	if (!this->vertex[0].IsValid() || !this->vertex[1].IsValid())
		return false;

	if (this->radius <= 0.0)
		return false;

	return true;
}

/*virtual*/ double CapsuleShape::CalcSize() const
{
	return(
		M_PI * this->radius * this->radius * (this->vertex[0] - this->vertex[1]).Length() +
		(4.0 / 3.0) * M_PI * this->radius * this->radius * this->radius
	);
}

void CapsuleShape::SetVertex(int i, const Vector3& point)
{
	if (i % 2 == 0)
		this->vertex[0] = point;
	else
		this->vertex[1] = point;
}

const Vector3& CapsuleShape::GetVertex(int i) const
{
	return (i % 2 == 0) ? this->vertex[0] : this->vertex[1];
}