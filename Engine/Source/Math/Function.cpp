#include "Function.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"

using namespace Imzadi;

//------------------------------------- Quadratic -------------------------------------

Quadratic::Quadratic()
{
	this->A = 0.0;
	this->B = 0.0;
	this->C = 0.0;
}

Quadratic::Quadratic(const Quadratic& quadratic)
{
	this->A = quadratic.A;
	this->B = quadratic.B;
	this->C = quadratic.C;
}

Quadratic::Quadratic(double A, double B, double C)
{
	this->A = A;
	this->B = B;
	this->C = C;
}

/*virtual*/ Quadratic::~Quadratic()
{
}

bool Quadratic::IsValid() const
{
	if (::isnan(this->A) || ::isinf(this->A))
		return false;

	if (::isnan(this->B) || ::isinf(this->B))
		return false;

	if (::isnan(this->C) || ::isinf(this->C))
		return false;

	return true;
}

/*virtual*/ double Quadratic::Evaluate(double x) const
{
	return this->A * x * x + this->B * x + this->C;
}

void Quadratic::Solve(std::vector<double>& realRoots) const
{
	realRoots.clear();
	double desc = this->Descriminant();
	if (desc == 0.0)
		realRoots.push_back(-this->B / (2.0 * this->A));
	else if (desc > 0.0)
	{
		double radical = ::sqrt(desc);
		realRoots.push_back((-this->B - radical) / (2.0 * this->A));
		realRoots.push_back((-this->B + radical) / (2.0 * this->A));
	}
}

double Quadratic::Descriminant() const
{
	return this->B * this->B - 4.0 * this->A * this->C;
}

bool Quadratic::FitToPoints(const Vector2& pointA, const Vector2& pointB, const Vector2& pointC)
{
	Matrix3x3 matrix;

	matrix.ele[0][0] = pointA.x * pointA.x;
	matrix.ele[0][1] = pointA.x;
	matrix.ele[0][2] = 1.0;

	matrix.ele[1][0] = pointB.x * pointB.x;
	matrix.ele[1][1] = pointB.x;
	matrix.ele[1][2] = 1.0;

	matrix.ele[2][0] = pointC.x * pointC.x;
	matrix.ele[2][1] = pointC.x;
	matrix.ele[2][2] = 1.0;

	Matrix3x3 matrixInv;
	if (!matrixInv.Invert(matrix))
		return false;

	Vector3 vector = matrixInv * Vector3(pointA.y, pointB.y, pointC.y);

	this->A = vector.x;
	this->B = vector.y;
	this->C = vector.z;

	return true;
}

void Quadratic::Dump(std::ostream& stream) const
{
	stream.write((char*)&this->A, sizeof(this->A));
	stream.write((char*)&this->B, sizeof(this->B));
	stream.write((char*)&this->C, sizeof(this->C));
}

void Quadratic::Restore(std::istream& stream)
{
	stream.read((char*)&this->A, sizeof(this->A));
	stream.read((char*)&this->B, sizeof(this->B));
	stream.read((char*)&this->C, sizeof(this->C));
}

//------------------------------------- Cubic -------------------------------------

Cubic::Cubic()
{
	this->A = 0.0;
	this->B = 0.0;
	this->C = 0.0;
	this->D = 0.0;
}

/*virtual*/ Cubic::~Cubic()
{
}

/*virtual*/ double Cubic::Evaluate(double x) const
{
	double sum = 0.0;
	sum += this->A;
	sum *= x;
	sum += this->B;
	sum *= x;
	sum += this->C;
	sum *= x;
	sum += this->D;
	return sum;
}

bool Cubic::FitToPoints(const Vector2& pointA, const Vector2& pointB, const Vector2& pointC, const Vector2& pointD)
{
	Matrix4x4 matrix;

	matrix.ele[0][0] = pointA.x * pointA.x * pointA.x;
	matrix.ele[0][1] = pointA.x * pointA.x;
	matrix.ele[0][1] = pointA.x;
	matrix.ele[0][2] = 1.0;

	matrix.ele[1][0] = pointB.x * pointB.x * pointB.x;
	matrix.ele[1][1] = pointB.x * pointB.x;
	matrix.ele[1][1] = pointB.x;
	matrix.ele[1][2] = 1.0;

	matrix.ele[2][0] = pointC.x * pointC.x * pointC.x;
	matrix.ele[2][1] = pointC.x * pointC.x;
	matrix.ele[2][1] = pointC.x;
	matrix.ele[2][2] = 1.0;

	matrix.ele[3][0] = pointD.x * pointD.x * pointD.x;
	matrix.ele[3][1] = pointD.x * pointD.x;
	matrix.ele[3][1] = pointD.x;
	matrix.ele[3][2] = 1.0;

	Matrix4x4 matrixInv;

	if (!matrixInv.Invert(matrix))
		return false;

	Vector4 vector = matrixInv * Vector4(pointA.y, pointB.y, pointC.y, pointD.y);

	this->A = vector.x;
	this->B = vector.y;
	this->C = vector.z;
	this->D = vector.w;

	return true;
}

void Cubic::Dump(std::ostream& stream) const
{
	stream.write((char*)&this->A, sizeof(this->A));
	stream.write((char*)&this->B, sizeof(this->B));
	stream.write((char*)&this->C, sizeof(this->C));
	stream.write((char*)&this->D, sizeof(this->D));
}

void Cubic::Restore(std::istream& stream)
{
	stream.read((char*)&this->A, sizeof(this->A));
	stream.read((char*)&this->B, sizeof(this->B));
	stream.read((char*)&this->C, sizeof(this->C));
	stream.read((char*)&this->D, sizeof(this->D));
}

//------------------------------------- QuadraticSpaceCurve -------------------------------------

QuadraticSpaceCurve::QuadraticSpaceCurve()
{
}

/*virtual*/ QuadraticSpaceCurve::~QuadraticSpaceCurve()
{
}

/*virtual*/ Vector3 QuadraticSpaceCurve::Evaluate(double t) const
{
	return Vector3(
		this->quadraticX.Evaluate(t),
		this->quadraticY.Evaluate(t),
		this->quadraticZ.Evaluate(t)
	);
}

bool QuadraticSpaceCurve::FitToPoints(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC, double tA, double tB, double tC)
{
	if (!this->quadraticX.FitToPoints(Vector2(tA, pointA.x), Vector2(tB, pointB.x), Vector2(tC, pointC.x)))
		return false;

	if (!this->quadraticY.FitToPoints(Vector2(tA, pointA.y), Vector2(tB, pointB.y), Vector2(tC, pointC.y)))
		return false;

	if (!this->quadraticZ.FitToPoints(Vector2(tA, pointA.z), Vector2(tB, pointB.z), Vector2(tC, pointC.z)))
		return false;

	return true;
}

void QuadraticSpaceCurve::Dump(std::ostream& stream) const
{
	this->quadraticX.Dump(stream);
	this->quadraticY.Dump(stream);
	this->quadraticZ.Dump(stream);
}

void QuadraticSpaceCurve::Restore(std::istream& stream)
{
	this->quadraticX.Restore(stream);
	this->quadraticY.Restore(stream);
	this->quadraticZ.Restore(stream);
}

//------------------------------------- CubicSpaceCurve -------------------------------------

CubicSpaceCurve::CubicSpaceCurve()
{
}

/*virtual*/ CubicSpaceCurve::~CubicSpaceCurve()
{
}

/*virtual*/ Vector3 CubicSpaceCurve::Evaluate(double t) const
{
	return Vector3(
		this->cubicX.Evaluate(t),
		this->cubicY.Evaluate(t),
		this->cubicZ.Evaluate(t)
	);
}

bool CubicSpaceCurve::FitToPoints(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC, const Vector3& pointD, double tA, double tB, double tC, double tD)
{
	if (!this->cubicX.FitToPoints(Vector2(tA, pointA.x), Vector2(tB, pointB.x), Vector2(tC, pointC.x), Vector2(tD, pointD.x)))
		return false;

	if (!this->cubicY.FitToPoints(Vector2(tA, pointA.y), Vector2(tB, pointB.y), Vector2(tC, pointC.y), Vector2(tD, pointD.y)))
		return false;

	if (!this->cubicZ.FitToPoints(Vector2(tA, pointA.z), Vector2(tB, pointB.z), Vector2(tC, pointC.z), Vector2(tD, pointD.z)))
		return false;

	return true;
}

void CubicSpaceCurve::Dump(std::ostream& stream) const
{
	this->cubicX.Dump(stream);
	this->cubicY.Dump(stream);
	this->cubicZ.Dump(stream);
}

void CubicSpaceCurve::Restore(std::istream& stream)
{
	this->cubicX.Restore(stream);
	this->cubicY.Restore(stream);
	this->cubicZ.Restore(stream);
}