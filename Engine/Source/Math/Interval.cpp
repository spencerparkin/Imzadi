#include "Interval.h"

using namespace Imzadi;

Interval::Interval()
{
	this->A = 0.0;
	this->B = 0.0;
}

Interval::Interval(const Interval& interval)
{
	this->A = interval.A;
	this->B = interval.B;
}

Interval::Interval(double A, double B)
{
	this->A = A;
	this->B = B;
}

/*virtual*/ Interval::~Interval()
{
}

void Interval::operator=(const Interval& interval)
{
	this->A = interval.A;
	this->B = interval.B;
}

bool Interval::operator==(const Interval& interval) const
{
	return this->A == interval.A && this->B == interval.B;
}

bool Interval::IsValid() const
{
	if (::isnan(this->A) || ::isinf(this->A))
		return false;

	if (::isnan(this->B) || ::isinf(this->B))
		return false;

	return this->A <= this->B;
}

bool Interval::ContainsValue(double value, double epsilon /*= 0.0*/) const
{
	return this->A - epsilon <= value && value <= this->B + epsilon;
}

bool Interval::ContainsInteriorValue(double value) const
{
	return this->A < value && value < this->B;
}

bool Interval::ContainsInterval(const Interval& interval) const
{
	return this->ContainsValue(interval.A) && this->ContainsValue(interval.B);
}

bool Interval::OverlapsWith(const Interval& interval) const
{
	Interval intersection;
	return intersection.Intersect(*this, interval);
}

bool Interval::Intersect(const Interval& intervalA, const Interval& intervalB)
{
	this->A = IMZADI_MAX(intervalA.A, intervalB.A);
	this->B = IMZADI_MIN(intervalA.B, intervalB.B);

	return this->IsValid();
}

void Interval::Subtract(const Interval& interval, std::vector<Interval>& intervalArray) const
{
	intervalArray.clear();
	if (*this == interval)
		return;

	Interval intersection;
	if (!intersection.Intersect(*this, interval))
	{
		intervalArray.push_back(*this);
		return;
	}

	if (interval == intersection)
	{
		intervalArray.push_back(Interval(this->A, interval.A));
		intervalArray.push_back(Interval(interval.B, this->B));
	}
	else if (this->A == intersection.A)
	{
		intervalArray.push_back(Interval(intersection.B, this->B));
	}
	else if (this->B == intersection.B)
	{
		intervalArray.push_back(Interval(this->A, intersection.A));
	}
	else
	{
		IMZADI_ASSERT(false);
	}
}

void Interval::Merge(const Interval& intervalA, const Interval& intervalB)
{
	this->A = IMZADI_MIN(intervalA.A, intervalB.A);
	this->B = IMZADI_MAX(intervalA.B, intervalB.B);
}

void Interval::Expand(double value)
{
	this->A = IMZADI_MIN(this->A, value);
	this->B = IMZADI_MAX(this->B, value);
}

void Interval::BoundValues(const std::vector<double>& valueArray)
{
	if (valueArray.size() == 0)
		return;

	this->A = valueArray[0];
	this->B = valueArray[0];

	for (int i = 1; i < (signed)valueArray.size(); i++)
		this->Expand(valueArray[i]);
}

double Interval::Center() const
{
	return (this->A + this->B) / 2.0;
}

double Interval::Size() const
{
	return this->B - this->A;
}

double Interval::Lerp(double alpha) const
{
	return this->A + (this->B - this->A) * alpha;
}

double Interval::Alpha(double value) const
{
	return (value - this->A) / (this->B - this->A);
}

bool Interval::Split(double alpha, Interval& intervalA, Interval& intervalB) const
{
	double lerpValue = this->Lerp(alpha);

	intervalA.A = this->A;
	intervalA.B = lerpValue;
	intervalB.A = lerpValue;
	intervalB.B = this->B;

	return intervalA.IsValid() && intervalB.IsValid();
}

double Interval::Random() const
{
	double alpha = double(rand()) / double(RAND_MAX);
	return this->Lerp(alpha);
}

void Interval::Dump(std::ostream& stream) const
{
	stream.write((char*)&this->A, sizeof(this->A));
	stream.write((char*)&this->B, sizeof(this->B));
}

void Interval::Restore(std::istream& stream)
{
	stream.read((char*)&this->A, sizeof(this->A));
	stream.read((char*)&this->B, sizeof(this->B));
}