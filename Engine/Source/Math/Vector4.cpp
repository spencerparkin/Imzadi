#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include <math.h>

using namespace Collision;

Vector4::Vector4()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
	this->w = 1.0;
}

Vector4::Vector4(const Vector3& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = 1.0;
}

Vector4::Vector4(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = 1.0;
}

Vector4::Vector4(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

/*virtual*/ Vector4::~Vector4()
{
}

Vector4& Vector4::operator=(const Vector4& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = vector.w;

	return *this;
}

Vector4& Vector4::operator=(const Vector3& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = 1.0;

	return *this;
}

void Vector4::operator+=(const Vector4& vector)
{
	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
	this->w += vector.w;
}

void Vector4::operator-=(const Vector4& vector)
{
	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
	this->w -= vector.w;
}

void Vector4::operator*=(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
	this->w *= scalar;
}

void Vector4::operator/=(double scalar)
{
	this->x /= scalar;
	this->y /= scalar;
	this->z /= scalar;
	this->w /= scalar;
}

void Vector4::SetComponents(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

void Vector4::GetComponents(double& x, double& y, double& z, double& w) const
{
	x = this->x;
	y = this->y;
	z = this->z;
	w = this->w;
}

void Vector4::Add(const Vector4& leftVector, const Vector4& rightVector)
{
	this->x = leftVector.x + rightVector.x;
	this->y = leftVector.y + rightVector.y;
	this->z = leftVector.z + rightVector.z;
	this->w = leftVector.w + rightVector.w;
}

void Vector4::Subtract(const Vector4& leftVector, const Vector4& rightVector)
{
	this->x = leftVector.x - rightVector.x;
	this->y = leftVector.y - rightVector.y;
	this->z = leftVector.z - rightVector.z;
	this->w = leftVector.w - rightVector.w;
}

/*static*/ double Vector4::Dot(const Vector4& leftVector, const Vector4& rightVector)
{
	double dot =
		leftVector.x * rightVector.x +
		leftVector.y * rightVector.y +
		leftVector.z * rightVector.z +
		leftVector.w * rightVector.w;

	return dot;
}

/*static*/ double Vector4::AngleBetween(const Vector4& vectorA, const Vector4& vectorB)
{
	double lengthA = vectorA.Length();
	double lengthB = vectorB.Length();
	double angle = acos(Vector4::Dot(vectorA, vectorB) / (lengthA * lengthB));
	return angle;
}

double Vector4::Length() const
{
	return sqrt(Vector4::Dot(*this, *this));
}

void Vector4::Scale(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
	this->w *= scalar;
}

bool Vector4::Homogenize()
{
	if (this->w == 0.0)
		return false;

	this->x /= this->w;
	this->y /= this->w;
	this->z /= this->w;
	this->w = 1.0;

	return true;
}

bool Vector4::Normalize()
{
	double length = this->Length();
	if (length == 0.0)
		return false;

	this->Scale(1.0 / length);
	return true;
}

void Vector4::Lerp(const Vector4& vectorA, const Vector4& vectorB, double alpha)
{
	*this = vectorA * (1.0 - alpha) + vectorB * alpha;
}

void Vector4::Slerp(const Vector4& vectorA, const Vector4& vectorB, double alpha)
{
	double angle = AngleBetween(vectorA, vectorB);
	if (angle == 0.0)
		this->Lerp(vectorA, vectorB, alpha);
	else
		*this = (vectorA * sin((1.0 - alpha) * angle) + vectorB * sin(alpha * angle)) / sin(angle);
}

bool Vector4::IsValid() const
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

void Vector4::Dump(std::ostream& stream) const
{
	stream.write((char*)&this->w, sizeof(this->w));
	stream.write((char*)&this->x, sizeof(this->x));
	stream.write((char*)&this->y, sizeof(this->y));
	stream.write((char*)&this->z, sizeof(this->z));
}

void Vector4::Restore(std::istream& stream)
{
	stream.read((char*)&this->w, sizeof(this->w));
	stream.read((char*)&this->x, sizeof(this->x));
	stream.read((char*)&this->y, sizeof(this->y));
	stream.read((char*)&this->z, sizeof(this->z));
}

namespace Collision
{
	Vector4 operator+(const Vector4& leftVector, const Vector4& rightVector)
	{
		Vector4 result;
		result.Add(leftVector, rightVector);
		return result;
	}

	Vector4 operator-(const Vector4& leftVector, const Vector4& rightVector)
	{
		Vector4 result;
		result.Subtract(leftVector, rightVector);
		return result;
	}

	Vector4 operator*(const Vector4& vector, double scalar)
	{
		Vector4 result(vector);
		result.Scale(scalar);
		return result;
	}

	Vector4 operator*(double scalar, const Vector4& vector)
	{
		Vector4 result(vector);
		result.Scale(scalar);
		return result;
	}

	Vector4 operator/(const Vector4& vector, double scalar)
	{
		Vector4 result(vector);
		result.Scale(1.0 / scalar);
		return result;
	}
}