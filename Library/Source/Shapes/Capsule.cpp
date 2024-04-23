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
	Vector3 pointA = this->objectToWorld.TransformPoint(this->lineSegment.point[0]);
	Vector3 pointB = this->objectToWorld.TransformPoint(this->lineSegment.point[1]);

	boundingBox.minCorner = pointA;
	boundingBox.maxCorner = pointA;

	Vector3 delta(this->radius, this->radius, this->radius);

	boundingBox.Expand(pointA + delta);
	boundingBox.Expand(pointA - delta);
	boundingBox.Expand(pointB + delta);
	boundingBox.Expand(pointB - delta);
}

/*virtual*/ bool CapsuleShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (::isnan(this->radius) || ::isinf(this->radius))
		return false;

	if (!this->lineSegment.point[0].IsValid() || !this->lineSegment.point[1].IsValid())
		return false;

	if (this->radius <= 0.0)
		return false;

	return true;
}

/*virtual*/ double CapsuleShape::CalcSize() const
{
	return(
		M_PI * this->radius * this->radius * this->lineSegment.Length() +
		(4.0 / 3.0) * M_PI * this->radius * this->radius * this->radius
	);
}

/*virtual*/ void CapsuleShape::DebugRender(DebugRenderResult* renderResult) const
{
}

void CapsuleShape::SetVertex(int i, const Vector3& point)
{
	if (i % 2 == 0)
		this->lineSegment.point[0] = point;
	else
		this->lineSegment.point[1] = point;
}

const Vector3& CapsuleShape::GetVertex(int i) const
{
	return (i % 2 == 0) ? this->lineSegment.point[0] : this->lineSegment.point[1];
}