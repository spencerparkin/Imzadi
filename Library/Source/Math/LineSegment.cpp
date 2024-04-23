#include "LineSegment.h"

using namespace Collision;

LineSegment::LineSegment()
{
}

LineSegment::LineSegment(const Vector3& pointA, const Vector3& pointB)
{
	this->point[0] = pointA;
	this->point[1] = pointB;
}

/*virtual*/ LineSegment::~LineSegment()
{
}

bool LineSegment::IsValid() const
{
	if (!this->point[0].IsValid())
		return false;

	if (!this->point[1].IsValid())
		return false;

	if (this->Length() == 0.0)
		return false;

	return true;
}

double LineSegment::Length() const
{
	return (this->point[1] - this->point[0]).Length();
}

double LineSegment::ShortestDistanceTo(const Vector3& point) const
{
	Vector3 lineVector = this->point[1] - this->point[0];
	double lineLength = lineVector.Length();
	Vector3 unitLineVector = lineVector / lineLength;
	Vector3 pointVector = point - this->point[0];
	double projectedLength = pointVector.Dot(unitLineVector);

	if (projectedLength <= 0.0)
		return pointVector.Length();
	else if (projectedLength >= lineLength)
		return (point - this->point[1]).Length();

	return (point - (this->point[0] + unitLineVector * projectedLength)).Length();
}

double LineSegment::ShortestDistanceTo(const LineSegment& lineSegment) const
{
	return 0.0;		// TODO: Write this.
}

double LineSegment::ShortestDistanceTo(const Plane& plane) const
{
	return 0.0;		// TODO: Write this.
}