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