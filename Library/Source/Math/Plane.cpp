#include "Plane.h"

using namespace Collision;

Plane::Plane()
{
	this->center = Vector3(0.0, 0.0, 0.0);
	this->unitNormal = Vector3(0.0, 0.0, 1.0);
}

Plane::Plane(const Vector3& point, const Vector3& unitNormal)
{
	this->center = point.ProjectedOnto(unitNormal);
	this->unitNormal = unitNormal;
}

Plane::Plane(const Plane& plane)
{
	this->center = plane.center;
	this->unitNormal = plane.unitNormal;
}

/*virtual*/ Plane::~Plane()
{
}

void Plane::operator=(const Plane& plane)
{
	this->center = plane.center;
	this->unitNormal = plane.unitNormal;
}

bool Plane::IsValid(double tolerance /*= 1e-7*/) const
{
	if (!this->center.IsValid())
		return false;

	if (!this->unitNormal.IsValid())
		return false;

	double length = this->unitNormal.Length();
	if (::fabs(length - 1.0) > tolerance)
		return false;

	return true;
}

double Plane::SignedDistanceTo(const Vector3& point) const
{
	return (point - this->center).Dot(this->unitNormal);
}

Plane::Side Plane::GetSide(const Vector3& point, double planeThickness /*= 0.0*/) const
{
	double signedDistance = this->SignedDistanceTo(point);
	if (::fabs(signedDistance) <= planeThickness)
		return Side::NEITHER;

	return signedDistance > 0.0 ? Side::FRONT : Side::BACK;
}

bool Plane::AllPointsOnSide(const std::vector<Vector3>& pointArray, Side side) const
{
	for (const Vector3& point : pointArray)
		if (side != this->GetSide(point))
			return false;

	return true;
}

bool Plane::AnyPointOnSide(const std::vector<Vector3>& pointArray, Side side) const
{
	for (const Vector3& point : pointArray)
		if (side == this->GetSide(point))
			return true;

	return false;
}