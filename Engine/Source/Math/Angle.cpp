#include "Angle.h"
#include <math.h>

using namespace Imzadi;

/*static*/ double Angle::Mod2Pi(double angle)
{
	angle = ::fmod(angle, 2.0 * M_PI);
	
	if (angle < 0.0)
		angle += 2.0 * M_PI;

	return angle;
}

/*static*/ double Angle::Distance(double angleA, double angleB)
{
	angleA = Mod2Pi(angleA);
	angleB = Mod2Pi(angleB);

	double angularDistance = ::fabs(angleB - angleA);
	if (angularDistance > M_PI)
		angularDistance = Opposing(angularDistance);

	return angularDistance;
}

/*static*/ double Angle::Complementary(double angle)
{
	return M_PI / 2.0 - angle;
}

/*static*/ double Angle::Supplementary(double angle)
{
	return M_PI - angle;
}

/*static*/ double Angle::Opposing(double angle)
{
	return 2.0 * M_PI - angle;
}

/*static*/ Angle::Type Angle::Classify(double angle)
{
	if (angle < M_PI / 2.0)
		return Type::ACUTE;
	else if (angle > M_PI / 2.0)
		return Type::OBTUSE;
	return Type::RIGHT;
}

/*static*/ double Angle::MakeClose(double angleA, double angleB)
{
	while (::fabs(angleB - angleA) > M_PI)
		angleA += 2.0 * M_PI * IMZADI_SIGN(angleB - angleA);

	return angleA;
}

/*static*/ double Angle::MoveTo(double angleA, double angleB, double stepSize)
{
	double angularDistance = Distance(angleA, angleB);
	if (angularDistance <= stepSize)
		return angleB;

	angleA = Mod2Pi(angleA);
	angleB = Mod2Pi(angleB);

	angleA = MakeClose(angleA, angleB);

	return angleA + stepSize * IMZADI_SIGN(angleB - angleA);
}

/*static*/ double Angle::RadiansToDegrees(double angleRadians)
{
	return IMZADI_RADS_TO_DEGS(angleRadians);
}

/*static*/ double Angle::DegreesToRadians(double angleDegrees)
{
	return IMZADI_DEGS_TO_RADS(angleDegrees);
}