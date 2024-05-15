#pragma once

#include "Defines.h"
#include <istream>
#include <ostream>

namespace Collision
{
	class Vector3;

	/*
	 * These are 4-dimensional vectors geared toward 3D computer graphics applications and homogenous coordinates.
	 */
	class COLLISION_LIB_API Vector4
	{
	public:
		Vector4();
		Vector4(const Vector3& vector);
		Vector4(double x, double y, double z);
		Vector4(double x, double y, double z, double w);
		virtual ~Vector4();

		Vector4& operator=(const Vector4& vector);
		Vector4& operator=(const Vector3& vector);
		void operator+=(const Vector4& vector);
		void operator-=(const Vector4& vector);
		void operator*=(double scalar);
		void operator/=(double scalar);

		bool IsValid() const;

		void Dump(std::ostream& stream) const;

		void Restore(std::istream& stream);

		void SetComponents(double x, double y, double z, double w);
		void GetComponents(double& x, double& y, double& z, double& w) const;

		void Add(const Vector4& leftVector, const Vector4& rightVector);
		void Subtract(const Vector4& leftVector, const Vector4& rightVector);

		static double Dot(const Vector4& leftVector, const Vector4& rightVector);

		static double AngleBetween(const Vector4& vectorA, const Vector4& vectorB);

		double Length() const;
		void Scale(double scalar);

		bool Homogenize();
		bool Normalize();

		void Lerp(const Vector4& vectorA, const Vector4& vectorB, double alpha);
		void Slerp(const Vector4& vectorA, const Vector4& vectorB, double alpha);

		union { double x, r; };
		union { double y, g; };
		union { double z, b; };
		union { double w, a; };
	};

	COLLISION_LIB_API Vector4 operator+(const Vector4& leftVector, const Vector4& rightVector);
	COLLISION_LIB_API Vector4 operator-(const Vector4& leftVector, const Vector4& rightVector);
	COLLISION_LIB_API Vector4 operator*(const Vector4& vector, double scalar);
	COLLISION_LIB_API Vector4 operator*(double scalar, const Vector4& vector);
	COLLISION_LIB_API Vector4 operator/(const Vector4& vector, double scalar);
}