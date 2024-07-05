#include "Vector3.h"

using namespace Imzadi;

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
	Vector3 xAxis = rejection;
	double length = 0.0;
	xAxis.Normalize(&length);
	Vector3 yAxis = unitAxis.Cross(xAxis);
	rejection = length * (::cos(angle) * xAxis + ::sin(angle) * yAxis);
	return projection + rejection;
}

void Vector3::SetAsOrthogonalTo(const Vector3& vector)
{
	double absX = ::fabs(vector.x);
	double absY = ::fabs(vector.y);
	double absZ = ::fabs(vector.z);

	if (absX <= absY && absX <= absZ)
	{
		this->x = 0.0;
		this->y = vector.z;
		this->z = -vector.y;
	}
	else if (absY <= absX && absY <= absZ)
	{
		this->x = vector.z;
		this->y = 0.0;
		this->z = -vector.x;
	}
	else
	{
		this->x = vector.y;
		this->y = -vector.x;
		this->z = 0.0;
	}
}

bool Vector3::IsPoint(const Vector3& point, double tolerance /*= 0.0*/) const
{
	return (*this - point).Length() <= tolerance;
}

bool Vector3::IsAnyPoint(const std::vector<Vector3>& pointArray, double tolerance /*= 0.0*/) const
{
	for (const Vector3& point : pointArray)
		if (point.IsPoint(*this, tolerance))
			return true;

	return false;
}

void Vector3::Dump(std::ostream& stream) const
{
	stream.write((char*)&this->x, sizeof(this->x));
	stream.write((char*)&this->y, sizeof(this->y));
	stream.write((char*)&this->z, sizeof(this->z));
}

void Vector3::Restore(std::istream& stream)
{
	stream.read((char*)&this->x, sizeof(this->x));
	stream.read((char*)&this->y, sizeof(this->y));
	stream.read((char*)&this->z, sizeof(this->z));
}

Vector3& Vector3::Lerp(const Vector3& vectorA, const Vector3& vectorB, double alpha)
{
	*this = vectorA + alpha * (vectorB - vectorA);
	return *this;
}

Vector3& Vector3::Slerp(const Vector3& unitVectorA, const Vector3& unitVectorB, double alpha)
{
	double angle = unitVectorA.AngleBetween(unitVectorB);
	*this = (::sin((1.0 - alpha) * angle) * unitVectorA + ::sin(alpha * angle) * unitVectorB) / ::sin(angle);
	return *this;
}

Vector3 Vector3::MoveTo(const Vector3& vector, double stepSize) const
{
	Vector3 delta = vector - *this;
	double distance = delta.Length();
	if (distance <= stepSize)
		return vector;
	return *this + delta * (stepSize / distance);
}