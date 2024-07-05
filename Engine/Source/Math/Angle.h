#pragma once

#include "Defines.h"

namespace Imzadi
{
	/**
	 * This class just contains some static methods that are useful when
	 * dealing with angles.  All methods use angles in radians unless
	 * otherwise stated.
	 */
	class IMZADI_API Angle
	{
	public:
		/**
		 * Return the equivilant angle in [0,2*pi).
		 */
		static double Mod2Pi(double angle);

		/**
		 * Calculate the angular distance between the two given angles.
		 * 
		 * @return The return value here will always be within [0,pi].
		 */
		static double Distance(double angleA, double angleB);

		/**
		 * Return pi/2 minus the given angle.
		 */
		static double Complementary(double angle);

		/**
		 * Return pi minus the given angle.
		 */
		static double Supplementary(double angle);

		/**
		 * Return 2*pi minus the given angle.
		 */
		static double Opposing(double angle);

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

		/**
		 * Return angle A plus a multiple of 2*pi such that the
		 * result is within pi of the angle B.
		 */
		static double MakeClose(double angleA, double angleB);

		/**
		 * Move angle A toward angle B by the given step-size, but
		 * without over-stepping.
		 * 
		 * @param[in] angleA This is the angle that is moved.
		 * @param[in] angleB This is the angle we're moving toward.
		 * @param[in] stepSize This is how far to move, unless it would cause us to over-step.  This must be non-negative.
		 * @return The moved angle is returned.
		 */
		static double MoveTo(double angleA, double angleB, double stepSize);

		/**
		 * Return the given angle (in radians) to degrees.
		 */
		static double RadiansToDegrees(double angleRadians);

		/**
		 * Return the given angle (in degrees) to radians.
		 */
		static double DegreesToRadians(double angleDegrees);
	};
}