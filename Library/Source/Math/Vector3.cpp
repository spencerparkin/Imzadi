#include "Vector3.h"

using namespace Collision;

bool Vector3::Normalize(double* length /*= nullptr*/)
{
	double scalar = 0.0;
	if (!length)
		length = &scalar;

	*length = this->Length();
	if (*length == 0.0)
		return false;

	double scale = 1.0 / *length;
	if (::isnan(scale) || scale != scale)
		return false;

	*this *= scale;
	return true;
}

Vector3 Vector3::Normalized() const
{
	return *this / this->Length();
}

Vector3 Vector3::ProjectedOnto(const Vector3& unitVector) const
{
	return unitVector * this->Dot(unitVector);
}

Vector3 Vector3::RejectedFrom(const Vector3& unitVector) const
{
	return *this - this->ProjectedOnto(unitVector);
}

Vector3 Vector3::Rotated(const Vector3& unitAxis, double angle) const
{
	Vector3 projection = this->ProjectedOnto(unitAxis);
	Vector3 rejection = *this - projection;

	Vector3 xAxis = rejection.Normalized();
	Vector3 yAxis = unitAxis.Cross(xAxis);

	rejection = ::cos(angle) * xAxis + ::sin(angle) * yAxis;
	return projection + rejection;
}