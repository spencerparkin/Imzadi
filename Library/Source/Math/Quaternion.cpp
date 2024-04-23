#include "Quaternion.h"
#include "Vector3.h"
#include "Plane.h"
#include "Ray.h"

using namespace Collision;

Quaternion::Quaternion()
{
	this->SetIdentity();
}

Quaternion::Quaternion(double w, double x, double y, double z)
{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

Quaternion::Quaternion(const Quaternion& quat)
{
	this->w = quat.w;
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
}

/*virtual*/ Quaternion::~Quaternion()
{
}

void Quaternion::operator=(const Quaternion& quat)
{
	this->w = quat.w;
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
}

void Quaternion::operator+=(const Quaternion& quat)
{
	this->w += quat.w;
	this->x += quat.x;
	this->y += quat.y;
	this->z += quat.z;
}

void Quaternion::operator-=(const Quaternion& quat)
{
	this->w -= quat.w;
	this->x -= quat.x;
	this->y -= quat.y;
	this->z -= quat.z;
}

void Quaternion::operator*=(const Quaternion& quat)
{
	*this = *this * quat;
}

void Quaternion::operator/=(const Quaternion& quat)
{
	*this = *this / quat;
}

bool Quaternion::IsValid() const
{
	if (::isnan(this->w) || ::isinf(this->w))
		return false;

	if (::isnan(this->x) || ::isinf(this->x))
		return false;

	if (::isnan(this->y) || ::isinf(this->y))
		return false;

	if (::isnan(this->z) || ::isinf(this->z))
		return false;

	return true;
}


void Quaternion::SetIdentity()
{
	this->w = 1.0;
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

void Quaternion::SetPoint(const Vector3& point)
{
	this->w = 0.0;
	this->x = point.x;
	this->y = point.y;
	this->z = point.z;
}

Vector3 Quaternion::GetPoint() const
{
	return Vector3(
		this->x,
		this->y,
		this->z
	);
}

void Quaternion::SetFromAxisAngle(const Vector3& unitAxis, double angle)
{
	double cosHalfAngle = ::cos(angle / 2.0);
	double sinHalfAngle = ::sin(angle / 2.0);

	this->w = cosHalfAngle;
	this->x = unitAxis.x * sinHalfAngle;
	this->y = unitAxis.y * sinHalfAngle;
	this->z = unitAxis.z * sinHalfAngle;
}

void Quaternion::GetToAxisAngle(Vector3& unitAxis, double& angle) const
{
	angle = ::acos(this->w) / 2.0;
	unitAxis = this->GetPoint().Normalized();
}

double Quaternion::SquareMagnitude() const
{
	return(
		this->w * this->w +
		this->x * this->x +
		this->y * this->y +
		this->z * this->z
	);
}

double Quaternion::Magnitude() const
{
	return ::sqrt(this->SquareMagnitude());
}

Quaternion Quaternion::Conjugated() const
{
	return Quaternion(
		this->w,
		-this->x,
		-this->y,
		-this->z
	);
}

Quaternion Quaternion::Inverted() const
{
	return this->Conjugated() / this->SquareMagnitude();
}

Quaternion Quaternion::Normalized() const
{
	return *this / this->Magnitude();
}

Vector3 Quaternion::Rotate(const Vector3& point) const
{
	Quaternion quatPoint;
	quatPoint.SetPoint(point);

	// All the math here could get boilded down to reduce the number of instructions, but this is fine for now.
	return (*this * quatPoint * this->Conjugated()).GetPoint();
}

Plane Quaternion::Rotate(const Plane& plane) const
{
	Plane rotatedPlane;
	rotatedPlane.center = this->Rotate(plane.center);
	rotatedPlane.unitNormal = this->Rotate(plane.unitNormal).Normalized();
	return rotatedPlane;
}

Ray Quaternion::Rotate(const Ray& ray) const
{
	Ray rotatedRay;
	rotatedRay.origin = this->Rotate(ray.origin);
	rotatedRay.unitDirection = this->Rotate(ray.unitDirection).Normalized();
	return rotatedRay;
}

namespace Collision
{
	Quaternion operator+(const Quaternion& quatA, const Quaternion& quatB)
	{
		return Quaternion(
			quatA.w + quatB.w,
			quatA.x + quatB.x,
			quatA.y + quatB.y,
			quatA.z + quatB.z
		);
	}

	Quaternion operator-(const Quaternion& quatA, const Quaternion& quatB)
	{
		return Quaternion(
			quatA.w - quatB.w,
			quatA.x - quatB.x,
			quatA.y - quatB.y,
			quatA.z - quatB.z
		);
	}

	Quaternion operator*(const Quaternion& quatA, const Quaternion& quatB)
	{
		return Quaternion(
			quatA.w * quatB.w -
			quatA.x * quatB.x -
			quatA.y * quatB.y -
			quatA.z * quatB.z,
			quatA.w * quatB.x +
			quatA.x * quatB.w +
			quatA.y * quatB.z -
			quatA.z * quatB.y,
			quatA.w * quatB.y -
			quatA.x * quatB.w +
			quatA.y * quatB.x +
			quatA.z * quatB.z,
			quatA.w * quatB.z +
			quatA.x * quatB.y -
			quatA.y * quatB.x +
			quatA.z * quatB.w
		);
	}

	Quaternion operator/(const Quaternion& quatA, const Quaternion& quatB)
	{
		return quatA * quatB.Inverted();
	}

	Quaternion operator*(const Quaternion& quat, double scalar)
	{
		return Quaternion(
			quat.w * scalar,
			quat.x * scalar,
			quat.y * scalar,
			quat.z * scalar
		);
	}

	Quaternion operator*(double scalar, const Quaternion& quat)
	{
		return Quaternion(
			quat.w * scalar,
			quat.x * scalar,
			quat.y * scalar,
			quat.z * scalar
		);
	}

	Quaternion operator/(const Quaternion& quat, double scalar)
	{
		return Quaternion(
			quat.w / scalar,
			quat.x / scalar,
			quat.y / scalar,
			quat.z / scalar
		);
	}
}