#include "Ray.h"
#include "Plane.h"
#include "AxisAlignedBoundingBox.h"
#include "LineSegment.h"
#include <algorithm>

using namespace Collision;

Ray::Ray()
{
}

Ray::Ray(const Vector3& point, const Vector3& unitDirection)
{
	this->origin = point;
	this->unitDirection = unitDirection;
}

Ray::Ray(const Ray& ray)
{
	this->origin = ray.origin;
	this->unitDirection = ray.unitDirection;
}

/*virtual*/ Ray::~Ray()
{
}

void Ray::operator=(const Ray& ray)
{
	this->origin = ray.origin;
	this->unitDirection = ray.unitDirection;
}

bool Ray::IsValid(double tolerance /*= 1e-7*/) const
{
	if (!this->origin.IsValid())
		return false;

	if (!this->unitDirection.IsValid())
		return false;

	double length = this->unitDirection.Length();
	if (::fabs(length - 1.0) > tolerance)
		return false;

	return true;
}

Vector3 Ray::CalculatePoint(double alpha) const
{
	return this->origin + this->unitDirection * alpha;
}

double Ray::CalculateAlpha(const Vector3& rayPoint) const
{
	return (rayPoint - this->origin).Dot(this->unitDirection);
}

double Ray::CastAgainst(const Plane& plane) const
{
	return (plane.center - this->origin).Dot(plane.unitNormal) / this->unitDirection.Dot(plane.unitNormal);
}

bool Ray::CastAgainst(const Plane& plane, double& alpha) const
{
	double numerator = (plane.center - this->origin).Dot(plane.unitNormal);
	double denominator = this->unitDirection.Dot(plane.unitNormal);

	alpha = numerator / denominator;
	if (::isnan(alpha) || ::isinf(alpha))
		return false;

	return alpha >= 0.0;
}

bool Ray::CastAgainst(const AxisAlignedBoundingBox& box, double& alpha) const
{
	std::vector<double> alphaArray;
	if (!this->CastAgainst(box, alphaArray))
		return false;

	alpha = alphaArray[0];
	return true;
}

bool Ray::CastAgainst(const AxisAlignedBoundingBox& box, std::vector<double>& alphaArray) const
{
	std::vector<Plane> sidePlaneArray;
	box.GetSidePlanes(sidePlaneArray);
	if (sidePlaneArray.size() == 0)
		return false;

	for (const Plane& sidePlane : sidePlaneArray)
	{
		double alpha = 0.0;
		if (this->CastAgainst(sidePlane, alpha))
		{
			Vector3 hitPoint = this->CalculatePoint(alpha);
			if (box.ContainsPoint(hitPoint, 1e-5))
				alphaArray.push_back(alpha);
		}
	}

	std::sort(alphaArray.begin(), alphaArray.end());
	return alphaArray.size() > 0;
}

bool Ray::HitsOrOriginatesIn(const AxisAlignedBoundingBox& box) const
{
	if (box.ContainsPoint(this->origin))
		return true;

	double alpha = 0.0;
	return this->CastAgainst(box, alpha);
}

void Ray::ToLineSegment(LineSegment& lineSegment, double alpha) const
{
	lineSegment.point[0] = this->origin;
	lineSegment.point[1] = this->CalculatePoint(alpha);
}

void Ray::FromLineSegment(const LineSegment& lineSegment)
{
	this->origin = lineSegment.point[0];
	this->unitDirection = lineSegment.GetDelta().Normalized();
}