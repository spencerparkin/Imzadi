#include "Matrix2x2.h"
#include "Vector2.h"

using namespace Collision;

Matrix2x2::Matrix2x2()
{
	this->SetIdentity();
}

Matrix2x2::Matrix2x2(const Matrix2x2& matrix)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

/*virtual*/ Matrix2x2::~Matrix2x2()
{
}

void Matrix2x2::operator=(const Matrix2x2& matrix)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix2x2::operator+=(const Matrix2x2& matrix)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] += matrix.ele[i][j];
}

void Matrix2x2::operator-=(const Matrix2x2& matrix)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] -= matrix.ele[i][j];
}

void Matrix2x2::operator*=(double scalar)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] *= scalar;
}

bool Matrix2x2::IsValid() const
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			double scalar = this->ele[i][j];

			if (::isnan(scalar) || ::isinf(scalar))
				return false;
		}
	}

	return true;
}

void Matrix2x2::SetIdentity()
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] = (i == j) ? 1.0 : 0.0;
}

void Matrix2x2::GetRowVectors(Vector2& xAxis, Vector2& yAxis) const
{
	xAxis.x = this->ele[0][0];
	xAxis.y = this->ele[0][1];

	yAxis.x = this->ele[1][0];
	yAxis.y = this->ele[1][1];
}

void Matrix2x2::SetRowVectors(const Vector2& xAxis, const Vector2& yAxis)
{
	this->ele[0][0] = xAxis.x;
	this->ele[0][1] = xAxis.y;

	this->ele[1][0] = yAxis.x;
	this->ele[1][1] = yAxis.y;
}

void Matrix2x2::GetColumnVectors(Vector2& xAxis, Vector2& yAxis) const
{
	xAxis.x = this->ele[0][0];
	yAxis.y = this->ele[1][0];

	yAxis.x = this->ele[0][1];
	yAxis.y = this->ele[1][1];
}

void Matrix2x2::SetColumnVectors(const Vector2& xAxis, const Vector2& yAxis)
{
	this->ele[0][0] = xAxis.x;
	this->ele[1][0] = xAxis.y;

	this->ele[0][1] = yAxis.x;
	this->ele[1][1] = yAxis.y;
}

Matrix2x2 Matrix2x2::Inverted() const
{
	Matrix2x2 matrix;
	matrix.Invert(*this);
	return matrix;
}

bool Matrix2x2::Invert(const Matrix2x2& matrix)
{
	double det = matrix.Determinant();
	if (det == 0.0)
		return false;

	double scalar = 1.0 / det;
	if (::isinf(scalar) || ::isnan(scalar))
		return false;

	this->ele[0][0] = matrix.ele[1][1] * scalar;
	this->ele[0][1] = -matrix.ele[0][1] * scalar;
	this->ele[1][0] = -matrix.ele[1][0] * scalar;
	this->ele[1][1] = matrix.ele[0][0] * scalar;

	return true;
}

Matrix2x2 Matrix2x2::Transposed() const
{
	Matrix2x2 matrix;
	matrix.Transpose(*this);
	return matrix;
}

void Matrix2x2::Transpose(const Matrix2x2& matrix)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			this->ele[i][j] = matrix.ele[j][i];
}

double Matrix2x2::Determinant() const
{
	return this->ele[0][0] * this->ele[1][1] - this->ele[0][1] * this->ele[1][0];
}

namespace Collision
{
	Matrix2x2 operator+(const Matrix2x2& matrixA, const Matrix2x2& matrixB)
	{
		Matrix2x2 sum;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				sum.ele[i][j] = matrixA.ele[i][j] + matrixB.ele[i][j];

		return sum;
	}

	Matrix2x2 operator-(const Matrix2x2& matrixA, const Matrix2x2& matrixB)
	{
		Matrix2x2 diff;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				diff.ele[i][j] = matrixA.ele[i][j] - matrixB.ele[i][j];

		return diff;
	}

	Matrix2x2 operator*(const Matrix2x2& matrixA, const Matrix2x2& matrixB)
	{
		Matrix2x2 product;

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				product.ele[i][j] =
					matrixA.ele[i][0] * matrixB.ele[0][j] +
					matrixA.ele[i][1] * matrixB.ele[1][j];
			}
		}

		return product;
	}

	Matrix2x2 operator/(const Matrix2x2& matrixA, const Matrix2x2& matrixB)
	{
		return matrixA * matrixB.Inverted();
	}

	Matrix2x2 operator*(const Matrix2x2& matrix, double scalar)
	{
		Matrix2x2 product;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				product.ele[i][j] = matrix.ele[i][j] * scalar;

		return product;
	}

	Matrix2x2 operator*(double scalar, const Matrix2x2& matrix)
	{
		Matrix2x2 product;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				product.ele[i][j] = matrix.ele[i][j] * scalar;

		return product;
	}

	Vector2 operator*(const Matrix2x2& matrix, const Vector2& vector)
	{
		return Vector2(
			vector.x * matrix.ele[0][0] + vector.y * matrix.ele[0][1],
			vector.x * matrix.ele[1][0] + vector.y * matrix.ele[1][1]
		);
	}

	Vector2 operator*(const Vector2& vector, const Matrix2x2& matrix)
	{
		return Vector2(
			vector.x * matrix.ele[0][0] + vector.y * matrix.ele[1][0],
			vector.x * matrix.ele[0][1] + vector.y * matrix.ele[1][1]
		);
	}
}