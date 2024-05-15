#include "Math/Matrix4x4.h"
#include "Math/Matrix3x3.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"
#include <math.h>

using namespace Collision;

Matrix4x4::Matrix4x4()
{
	this->Identity();
}

Matrix4x4::Matrix4x4(const Matrix4x4& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

Matrix4x4::Matrix4x4(const Matrix3x3& matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i < 3 && j < 3)
				this->ele[i][j] = matrix.ele[i][j];
			else
				this->ele[i][j] = (i == j) ? 1.0 : 0.0;
		}
	}
}

Matrix4x4::Matrix4x4(const Matrix3x3& matrix, const Vector3& translation)
{
	this->ele[0][0] = matrix.ele[0][0];
	this->ele[1][0] = matrix.ele[1][0];
	this->ele[2][0] = matrix.ele[2][0];

	this->ele[0][1] = matrix.ele[0][1];
	this->ele[1][1] = matrix.ele[1][1];
	this->ele[2][1] = matrix.ele[2][1];

	this->ele[0][2] = matrix.ele[0][2];
	this->ele[1][2] = matrix.ele[1][2];
	this->ele[2][2] = matrix.ele[2][2];

	this->ele[0][3] = translation.x;
	this->ele[1][3] = translation.y;
	this->ele[2][3] = translation.z;

	this->ele[3][0] = 0.0;
	this->ele[3][1] = 0.0;
	this->ele[3][2] = 0.0;

	this->ele[3][3] = 1.0;
}

/*virtual*/ Matrix4x4::~Matrix4x4()
{
}

void Matrix4x4::operator=(const Matrix4x4& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix4x4::operator=(const Matrix3x3& matrix)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix4x4::Identity()
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = (i == j) ? 1.0 : 0.0;
}

bool Matrix4x4::SetCol(int col, const Vector3& vector)
{
	if (col < 0 || col > 3)
		return false;

	this->ele[0][col] = vector.x;
	this->ele[1][col] = vector.y;
	this->ele[2][col] = vector.z;
	return true;
}

bool Matrix4x4::GetCol(int col, Vector3& vector) const
{
	if (col < 0 || col > 3)
		return false;

	vector.x = this->ele[0][col];
	vector.y = this->ele[1][col];
	vector.z = this->ele[2][col];
	return true;
}

void Matrix4x4::GetAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const
{
	this->GetCol(0, xAxis);
	this->GetCol(1, yAxis);
	this->GetCol(2, zAxis);
}

void Matrix4x4::SetAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
{
	this->SetCol(0, xAxis);
	this->SetCol(1, yAxis);
	this->SetCol(2, zAxis);
}

Vector3 Matrix4x4::TransformVector(const Vector3& vector) const
{
	Vector3 vectorTransformed;

	vectorTransformed.SetComponents(
		vector.x * this->ele[0][0] +
		vector.y * this->ele[0][1] +
		vector.z * this->ele[0][2],
		vector.x * this->ele[1][0] +
		vector.y * this->ele[1][1] +
		vector.z * this->ele[1][2],
		vector.x * this->ele[2][0] +
		vector.y * this->ele[2][1] +
		vector.z * this->ele[2][2]);
	
	return vectorTransformed;
}

Vector3 Matrix4x4::TransformPoint(const Vector3& point) const
{
	Vector3 pointTransformed;

	pointTransformed.SetComponents(
		point.x * this->ele[0][0] +
		point.y * this->ele[0][1] +
		point.z * this->ele[0][2] +
		this->ele[0][3],
		point.x * this->ele[1][0] +
		point.y * this->ele[1][1] +
		point.z * this->ele[1][2] +
		this->ele[1][3],
		point.x * this->ele[2][0] +
		point.y * this->ele[2][1] +
		point.z * this->ele[2][2] +
		this->ele[2][3]);

	double w = 
		point.x * this->ele[3][0] +
		point.y * this->ele[3][1] +
		point.z * this->ele[3][2] +
		this->ele[3][3];

	if (w != 1.0)
	{
		pointTransformed.x /= w;
		pointTransformed.y /= w;
		pointTransformed.z /= w;
	}

	return pointTransformed;
}

Vector4 Matrix4x4::TransformVector(const Vector4& vector) const
{
	Vector4 vectorTransformed;

	vectorTransformed.SetComponents(
		this->ele[0][0] * vector.x +
		this->ele[0][1] * vector.y +
		this->ele[0][2] * vector.z +
		this->ele[0][3] * vector.w,
		this->ele[1][0] * vector.x +
		this->ele[1][1] * vector.y +
		this->ele[1][2] * vector.z +
		this->ele[1][3] * vector.w,
		this->ele[2][0] * vector.x +
		this->ele[2][1] * vector.y +
		this->ele[2][2] * vector.z +
		this->ele[2][3] * vector.w,
		this->ele[3][0] * vector.x +
		this->ele[3][1] * vector.y +
		this->ele[3][2] * vector.z +
		this->ele[3][3] * vector.w);

	return vectorTransformed;
}

void Matrix4x4::Multiply(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			this->ele[i][j] =
				leftMatrix.ele[i][0] * rightMatrix.ele[0][j] +
				leftMatrix.ele[i][1] * rightMatrix.ele[1][j] +
				leftMatrix.ele[i][2] * rightMatrix.ele[2][j] +
				leftMatrix.ele[i][3] * rightMatrix.ele[3][j];
		}
	}
}

bool Matrix4x4::Divide(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix)
{
	Matrix4x4 rightMatrixInverted;
	if (!rightMatrixInverted.Invert(rightMatrix))
		return false;

	this->Multiply(leftMatrix, rightMatrixInverted);
	return true;
}

bool Matrix4x4::Invert(const Matrix4x4& matrix)
{
	double det = matrix.Determinant();
	if (det == 0.0)
		return false;

	this->ele[0][0] = (matrix.ele[1][1] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[1][2] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) + matrix.ele[1][3] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2])) / det;
	this->ele[0][1] = -(matrix.ele[0][1] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[0][2] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) + matrix.ele[0][3] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2])) / det;
	this->ele[0][2] = (matrix.ele[0][1] * (matrix.ele[1][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[1][2])) / det;
	this->ele[0][3] = -(matrix.ele[0][1] * (matrix.ele[1][2] * matrix.ele[2][3] - matrix.ele[2][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][1] * matrix.ele[2][3] - matrix.ele[2][1] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][1] * matrix.ele[2][2] - matrix.ele[2][1] * matrix.ele[1][2])) / det;
	this->ele[1][0] = -(matrix.ele[1][0] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[1][2] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[1][3] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2])) / det;
	this->ele[1][1] = (matrix.ele[0][0] * (matrix.ele[2][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[2][3]) - matrix.ele[0][2] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[0][3] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2])) / det;
	this->ele[1][2] = -(matrix.ele[0][0] * (matrix.ele[1][2] * matrix.ele[3][3] - matrix.ele[3][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[1][2])) / det;
	this->ele[1][3] = (matrix.ele[0][0] * (matrix.ele[1][2] * matrix.ele[2][3] - matrix.ele[2][2] * matrix.ele[1][3]) - matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[2][3] - matrix.ele[2][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[2][2] - matrix.ele[2][0] * matrix.ele[1][2])) / det;
	this->ele[2][0] = (matrix.ele[1][0] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) - matrix.ele[1][1] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[1][3] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[2][1] = -(matrix.ele[0][0] * (matrix.ele[2][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[2][3]) - matrix.ele[0][1] * (matrix.ele[2][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[2][3]) + matrix.ele[0][3] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[2][2] = (matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[3][3] - matrix.ele[3][1] * matrix.ele[1][3]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[3][3] - matrix.ele[3][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[1][1])) / det;
	this->ele[2][3] = -(matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[2][3] - matrix.ele[2][1] * matrix.ele[1][3]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[2][3] - matrix.ele[2][0] * matrix.ele[1][3]) + matrix.ele[0][3] * (matrix.ele[1][0] * matrix.ele[2][1] - matrix.ele[2][0] * matrix.ele[1][1])) / det;
	this->ele[3][0] = -(matrix.ele[1][0] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2]) - matrix.ele[1][1] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2]) + matrix.ele[1][2] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[3][1] = (matrix.ele[0][0] * (matrix.ele[2][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[2][2]) - matrix.ele[0][1] * (matrix.ele[2][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[2][2]) + matrix.ele[0][2] * (matrix.ele[2][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[2][1])) / det;
	this->ele[3][2] = -(matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[3][2] - matrix.ele[3][1] * matrix.ele[1][2]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[3][2] - matrix.ele[3][0] * matrix.ele[1][2]) + matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[3][1] - matrix.ele[3][0] * matrix.ele[1][1])) / det;
	this->ele[3][3] = (matrix.ele[0][0] * (matrix.ele[1][1] * matrix.ele[2][2] - matrix.ele[2][1] * matrix.ele[1][2]) - matrix.ele[0][1] * (matrix.ele[1][0] * matrix.ele[2][2] - matrix.ele[2][0] * matrix.ele[1][2]) + matrix.ele[0][2] * (matrix.ele[1][0] * matrix.ele[2][1] - matrix.ele[2][0] * matrix.ele[1][1])) / det;

	return true;
}

void Matrix4x4::Transpose(const Matrix4x4& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[j][i];
}

double Matrix4x4::Determinant() const
{
	double det =
			this->ele[0][0] * (
			this->ele[1][1] * (
				this->ele[2][2] * this->ele[3][3] - this->ele[3][2] * this->ele[2][3]
			) - this->ele[1][2] * (
				this->ele[2][1] * this->ele[3][3] - this->ele[3][1] * this->ele[2][3]
			) + this->ele[1][3] * (
				this->ele[2][1] * this->ele[3][2] - this->ele[3][1] * this->ele[2][2]
			)
		) - this->ele[0][1] * (
			this->ele[1][0] * (
				this->ele[2][2] * this->ele[3][3] - this->ele[3][2] * this->ele[2][3]
			) - this->ele[1][2] * (
				this->ele[2][0] * this->ele[3][3] - this->ele[3][0] * this->ele[2][3]
			) + this->ele[1][3] * (
				this->ele[2][0] * this->ele[3][2] - this->ele[3][0] * this->ele[2][2]
			)
		) + this->ele[0][2] * (
			this->ele[1][0] * (
				this->ele[2][1] * this->ele[3][3] - this->ele[3][1] * this->ele[2][3]
			) - this->ele[1][1] * (
				this->ele[2][0] * this->ele[3][3] - this->ele[3][0] * this->ele[2][3]
			) + this->ele[1][3] * (
				this->ele[2][0] * this->ele[3][1] - this->ele[3][0] * this->ele[2][1]
			)
		) - this->ele[0][3] * (
			this->ele[1][0] * (
				this->ele[2][1] * this->ele[3][2] - this->ele[3][1] * this->ele[2][2]
			) - this->ele[1][1] * (
				this->ele[2][0] * this->ele[3][2] - this->ele[3][0] * this->ele[2][2]
			) + this->ele[1][2] * (
				this->ele[2][0] * this->ele[3][1] - this->ele[3][0] * this->ele[2][1]
			)
		);

	return det;
}

void Matrix4x4::SetTranslation(const Vector3& translation)
{
	this->ele[0][3] = translation.x;
	this->ele[1][3] = translation.y;
	this->ele[2][3] = translation.z;
}

void Matrix4x4::SetScale(const Vector3& scale)
{
	this->SetCol(0, Vector3(scale.x, 0.0, 0.0));
	this->SetCol(1, Vector3(0.0, scale.y, 0.0));
	this->SetCol(2, Vector3(0.0, 0.0, scale.z));
}

void Matrix4x4::SetUniformScale(double scale)
{
	this->SetCol(0, Vector3(scale, 0.0, 0.0));
	this->SetCol(1, Vector3(0.0, scale, 0.0));
	this->SetCol(2, Vector3(0.0, 0.0, scale));
}

void Matrix4x4::SetRotation(const Vector3& axis, double angle)
{
	Vector3 xAxis(1.0, 0.0, 0.0);
	Vector3 yAxis(0.0, 1.0, 0.0);
	Vector3 zAxis(0.0, 0.0, 1.0);

	Vector3 xAxisRotated, yAxisRotated, zAxisRotated;

	xAxisRotated = xAxis.Rotated(axis, angle);
	yAxisRotated = yAxis.Rotated(axis, angle);
	zAxisRotated = zAxis.Rotated(axis, angle);

	this->SetCol(0, xAxisRotated);
	this->SetCol(1, yAxisRotated);
	this->SetCol(2, zAxisRotated);
}

void Matrix4x4::RigidBodyMotion(const Vector3& axis, double angle, const Vector3& delta)
{
	this->Identity();
	this->SetRotation(axis, angle);
	this->SetTranslation(delta);
}

void Matrix4x4::RigidBodyMotion(const Quaternion& quat, const Vector3& delta)
{
	Vector3 axis;
	double angle;
	quat.GetToAxisAngle(axis, angle);
	this->RigidBodyMotion(axis, angle, delta);
}

bool Matrix4x4::OrthonormalizeOrientation()
{
	Vector3 xAxis, yAxis, zAxis;
	this->GetAxes(xAxis, yAxis, zAxis);

	if (!xAxis.Normalize())
		return false;

	yAxis = yAxis.RejectedFrom(xAxis);

	if (!yAxis.Normalize())
		return false;

	zAxis.Cross(xAxis, yAxis);
	zAxis.Normalize();

	this->SetAxes(xAxis, yAxis, zAxis);
	return true;
}

void Matrix4x4::Dump(std::ostream& stream) const
{
	// TODO: Write this.
}

void Matrix4x4::Restore(std::istream& stream)
{
	// TODO: Write this.
}

namespace Collision
{
	Matrix4x4 operator*(const Matrix4x4& leftMatrix, const Matrix4x4& rightMatrix)
	{
		Matrix4x4 product;
		product.Multiply(leftMatrix, rightMatrix);
		return product;
	}
}