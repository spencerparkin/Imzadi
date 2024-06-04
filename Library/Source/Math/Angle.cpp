#include "Angle.h"
#include <math.h>

using namespace Collision;

/*static*/ void Angle::MakeClose(double& angleA, double angleB)
{
	angleA = ::fmod(angleA, 2.0 * M_PI);
	angleB = ::fmod(angleB, 2.0 * M_PI);

	while (true)
	{
		double distance = ::fabs(angleA - angleB);
		if (distance <= M_PI)
			break;

		angleA += 2.0 * M_PI * COLL_SYS_SIGN(angleB - angleA);
	}
}