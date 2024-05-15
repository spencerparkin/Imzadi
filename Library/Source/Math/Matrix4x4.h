#pragma once

#include "Defines.h"

namespace Collision
{
	class Vector3;
	class Vector4;
	class Matrix3x3;
	class Quaternion;
	class Frustum;

	/**
	 * These are 4x4 matrices geared toward 3D computer graphics applications.
	 */
	class COLLISION_LIB_API Matrix4x4
	{
	public:
		Matrix4x4();
		Matrix4x4(const Matrix4x4& matrix);
		Matrix4x4(const Matrix3x3& matrix);
		Matrix4x4(const Matrix3x3& matrix, const Vector3& translation);
		virtual ~Matrix4x4();

		void Identity();

		bool SetCol(int col, const Vector3& vector);
		bool GetCol(int col, Vector3& vector) const;

		void GetAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;
		void SetAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

		// Note that if shear or non-uniform scale exist in a matrix, then you need to use
		// the inverse transpose of a matrix in order to transform vectors.
		Vector3 TransformVector(const Vector3& vector) const;
		Vector3 TransformPoint(const Vector3& point) const;
		Vector4 TransformVector(const Vector4& vector) const;

		void Multiply(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);
		bool Divide(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);

		bool Invert(const Matrix4x4& matrix);
		void Transpose(const Matrix4x4& matrix);

		double Determinant() const;

		void RigidBodyMotion(const Vector3& axis, double angle, const Vector3& delta);
		void RigidBodyMotion(const Quaternion& quat, const Vector3& delta);

		void SetTranslation(const Vector3& translation);
		void SetScale(const Vector3& scale);
		void SetUniformScale(double scale);
		void SetRotation(const Vector3& axis, double angle);

		bool OrthonormalizeOrientation();

		void operator=(const Matrix4x4& matrix);
		void operator=(const Matrix3x3& matrix);

		double ele[4][4];
	};

	COLLISION_LIB_API Matrix4x4 operator*(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix);
}