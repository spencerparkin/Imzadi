#include "LineSegment.h"
#include "Plane.h"
#include "Matrix2x2.h"
#include "Vector2.h"

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
	return this->GetDelta().Length();
}

Vector3 LineSegment::ClosestPointTo(const Vector3& point) const
{
	Vector3 lineVector = this->GetDelta();
	double lineLength = lineVector.Length();
	Vector3 unitLineVector = lineVector / lineLength;
	Vector3 pointVector = point - this->point[0];
	double projectedLength = pointVector.Dot(unitLineVector);

	if (projectedLength <= 0.0)
		return this->point[0];
	else if (projectedLength >= lineLength)
		return this->point[1];

	return this->point[0] + unitLineVector * projectedLength;
}

double LineSegment::ShortestDistanceTo(const Vector3& point) const
{
	return (point - this->ClosestPointTo(point)).Length();
}

double LineSegment::ShortestDistanceTo(const LineSegment& lineSegment) const
{
	LineSegment shortestConnector;
	if (shortestConnector.SetAsShortestConnector(*this, lineSegment))
		return shortestConnector.Length();

	// TODO: We shouldn't actually fail in this case, because there is a correct answer; we're just not getting it.
	return 0.0;
}

double LineSegment::ShortestDistanceTo(const Plane& plane) const
{
	double distanceA = plane.SignedDistanceTo(this->point[0]);
	double distanceB = plane.SignedDistanceTo(this->point[1]);

	if (COLL_SYS_SIGN(distanceA) != COLL_SYS_SIGN(distanceB))
		return 0.0;

	return COLL_SYS_MIN(::fabs(distanceA), ::fabs(distanceB));
}

Vector3 LineSegment::Lerp(double lambda) const
{
	return this->point[0] + lambda * this->GetDelta();
}

bool LineSegment::SetAsShortestConnector(const LineSegment& lineSegmentA, const LineSegment& lineSegmentB)
{
	Vector3 a = lineSegmentA.point[0];
	Vector3 b = lineSegmentA.GetDelta();
	Vector3 c = lineSegmentB.point[0];
	Vector3 d = lineSegmentB.GetDelta();

	Matrix2x2 matrix;
	matrix.ele[0][0] = b.Dot(b);
	matrix.ele[0][1] = -b.Dot(d);
	matrix.ele[1][0] = matrix.ele[0][1];
	matrix.ele[1][1] = d.Dot(d);

	Matrix2x2 matrixInv;
	if (!matrixInv.Invert(matrix))
		return false;

	Vector3 delta = a - c;
	Vector2 vector;
	vector.x = b.Dot(delta);
	vector.y = -d.Dot(delta);
	vector = matrixInv * vector;

	double alpha = vector.x;
	double beta = vector.y;
	double lengthA = lineSegmentA.Length();
	double lengthB = lineSegmentB.Length();

	alpha = COLL_SYS_MIN(alpha, lengthA);
	alpha = COLL_SYS_MAX(alpha, 0.0);
	beta = COLL_SYS_MIN(beta, lengthB);
	beta = COLL_SYS_MAX(beta, 0.0);

	this->point[0] = lineSegmentA.Lerp(alpha);
	this->point[1] = lineSegmentB.Lerp(beta);

	return true;
}

Vector3 LineSegment::GetDelta() const
{
	return this->point[1] - this->point[0];
}