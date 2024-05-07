#include "Vector2.h"

using namespace Collision;

void Vector2::SetFromPolarCoords(double angle, double radius)
{
	this->x = radius * ::cos(angle);
	this->y = radius * ::sin(angle);
}

void Vector2::GetToPolarCoords(double& angle, double& radius) const
{
	radius = this->Length();
	angle = ::atan2(this->y, this->x);
}

Vector2 Vector2::Normalized() const
{
	return *this / this->Length();
}

bool Vector2::Normalize(double* length /*= nullptr*/)
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

void Vector2::Dump(std::ostream& stream) const
{
	stream.write((char*)&this->x, sizeof(this->x));
	stream.write((char*)&this->y, sizeof(this->y));
}

void Vector2::Restore(std::istream& stream)
{
	stream.read((char*)&this->x, sizeof(this->x));
	stream.read((char*)&this->y, sizeof(this->y));
}