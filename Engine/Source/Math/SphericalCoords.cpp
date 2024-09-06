#include "SphericalCoords.h"
#include "Vector3.h"

using namespace Imzadi;

SphericalCoords::SphericalCoords()
{
	this->radius = 0.0;
	this->longitudeAngle = 0.0;
	this->latitudeAngle = 0.0;
}

SphericalCoords::SphericalCoords(double radius, double longitudeAngle, double latitudeAngle)
{
	this->radius = radius;
	this->longitudeAngle = longitudeAngle;
	this->latitudeAngle = latitudeAngle;
}

SphericalCoords::SphericalCoords(const SphericalCoords& sphericalCoords)
{
	this->radius = sphericalCoords.radius;
	this->longitudeAngle = sphericalCoords.longitudeAngle;
	this->latitudeAngle = sphericalCoords.latitudeAngle;
}

/*virtual*/ SphericalCoords::~SphericalCoords()
{
}

void SphericalCoords::operator=(const SphericalCoords& sphericalCoords)
{
	this->radius = sphericalCoords.radius;
	this->longitudeAngle = sphericalCoords.longitudeAngle;
	this->latitudeAngle = sphericalCoords.latitudeAngle;
}

void SphericalCoords::SetFromVector(const Vector3& vector)
{
	Vector3 unitVector(vector);
	if (!unitVector.Normalize(&this->radius))
	{
		this->radius = 0.0;
		this->longitudeAngle = 0.0;
		this->latitudeAngle = 0.0;
	}
	else
	{
		this->latitudeAngle = ::asin(unitVector.y);
		double cosLattitudeAngle = ::cos(this->latitudeAngle);
		double cosLongitudeAngle = unitVector.x / cosLattitudeAngle;
		double sinLongitudeAngle = unitVector.z / cosLattitudeAngle;
		this->longitudeAngle = ::atan2(sinLongitudeAngle, cosLongitudeAngle);
	}
}

Vector3 SphericalCoords::GetToVector() const
{
	Vector3 point;

	double cosLongitudeAngle = ::cos(this->longitudeAngle);
	double sinLongitudeAngle = ::sin(this->longitudeAngle);

	double cosLattitudeAngle = ::cos(this->latitudeAngle);
	double sinLattitudeAngle = ::sin(this->latitudeAngle);

	point.x = this->radius * cosLattitudeAngle * cosLongitudeAngle;
	point.y = this->radius * sinLattitudeAngle;
	point.z = this->radius * cosLattitudeAngle * sinLongitudeAngle;

	return point;
}

SphericalCoords& SphericalCoords::Lerp(const SphericalCoords& coordsA, const SphericalCoords& coordsB, double alpha)
{
	this->radius = coordsA.radius + alpha * (coordsB.radius - coordsA.radius);
	this->latitudeAngle = coordsA.latitudeAngle + alpha * (coordsB.latitudeAngle - coordsA.latitudeAngle);
	this->longitudeAngle = coordsA.longitudeAngle + alpha * (coordsB.longitudeAngle - coordsA.longitudeAngle);
	return *this;
}