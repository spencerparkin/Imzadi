#include "Quadratic.h"

using namespace Collision;

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

double Quadratic::Evaluate(double x) const
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