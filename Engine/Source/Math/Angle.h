#pragma once

#include "Defines.h"

namespace Imzadi
{
	/**
	 * This class just contains some static methods that are useful when
	 * dealing with angles (in radians.)
	 */
	class IMZADI_API Angle
	{
	public:
		/**
		 * Move the first angle as close to the second in multiples of 2*pi.
		 *
		 * @param[in,out] angleA A multiple of 2*pi is added to this angle.
		 * @param[in] angleB This is the target angle.
		 */
		static void MakeClose(double& angleA, double angleB);

		enum Type
		{
			ACUTE,
			OBTUSE,
			RIGHT
		};

		/**
		 * Tell the caler whether the angle is less than 90 degrees (acute),
		 * greater than 90 degrees (obtuse), or a right angle.
		 */
		static Type Classify(double angle);
	};
}