#pragma once

#include "Defines.h"
#include <vector>
#include <ostream>
#include <istream>

namespace Imzadi
{
	/**
	 * These are closed intervals (i.e., [A,B] where A and B are finite real values.)
	 * The result of any method of this class is left undefined if this or a
	 * given interval is invalid.
	 */
	class IMZADI_API Interval
	{
	public:
		/**
		 * Construct the interval [0,0].
		 */
		Interval();

		/**
		 * Construct an interval that is a copy of the given interval.
		 */
		Interval(const Interval& interval);

		/**
		 * Construct an interval with the given boundary values.
		 */
		Interval(double A, double B);

		/**
		 * Do nothing.
		 */
		virtual ~Interval();

		/**
		 * Assign the boundary values of this interval to be that of the given interval.
		 */
		void operator=(const Interval& interval);

		/**
		 * Return true if and only if this interval has the same boundary points as the given interval.
		 */
		bool operator==(const Interval& interval) const;

		/**
		 * Not only check for NaN and Inf, but make sure that A <= B, where
		 * [A,B] denotes this interval.
		 */
		bool IsValid() const;

		/**
		 * Return true if the given value V is contained within this interval [A,B]; false, otherwise.
		 * That is, return A - E <= V <= B + E, where E is the given epsilon.
		 */
		bool ContainsValue(double value, double epsilon = 0.0) const;

		/**
		 * Return true if the given value V is contained within the open interval (A,B); false, otherwise.
		 * Here, A and B are the boundary points of this (closed) interval [A,B].
		 * In other words, return A < V < B.
		 */
		bool ContainsInteriorValue(double value) const;

		/**
		 * Return true if the given interval is a sub-interval of this interval.
		 * That is, return true if the boundary points of the given interval are
		 * contained within this interval.
		 */
		bool ContainsInterval(const Interval& interval) const;

		/**
		 * Return true if and only if there exists a value in the given
		 * interval that also exists in this interval.
		 */
		bool OverlapsWith(const Interval& interval) const;

		/**
		 * Make this interval the intersection, if any, of the two given intervals.
		 * False is returned if the two given intervals do not overlap, and this
		 * interval is left undefined.
		 */
		bool Intersect(const Interval& intervalA, const Interval& intervalB);

		/**
		 * Subtract from this interval the given interval and return the
		 * result in the given interval array.  Zero, one or two intervals
		 * may be returned.
		 */
		void Subtract(const Interval& interval, std::vector<Interval>& intervalArray) const;

		/**
		 * Make this interval the smallest possible interval that can
		 * contain the two given intervals.
		 */
		void Merge(const Interval& intervalA, const Interval& intervalB);

		/**
		 * Minimally expand this interval so that it contains the given value.
		 */
		void Expand(double value);

		/**
		 * Make this interval the smallest possible itnerval that can
		 * contain the given set of values.
		 */
		void BoundValues(const std::vector<double>& valueArray);

		/**
		 * Return the mid-point of this interval.  That is, if this interval
		 * is [A,B], return (A+B)/2.
		 */
		double Center() const;

		/**
		 * Return B-A where this interval is denoted [A,B].
		 */
		double Size() const;

		/**
		 * Calculate and return the linear interpolation of this
		 * intervals boundary values.  The returned value is in this
		 * interval if and only if the given alpha value is in [0,1].
		 */
		double Lerp(double alpha) const;

		/**
		 * Calculate and return the linear interpolation amount
		 * needed to get the given value with such an interpolation.
		 * This is a valid in [0,1] if the given value is in range [A,B].
		 */
		double Alpha(double value) const;

		/**
		 * Split this interval into two intervals by the given alpha value,
		 * used to linearly interpolate between this interval's boundary
		 * values.  If the given alpha value is not in [0,1], false is returned;
		 * true, otherwise.  The first returned interval is always left of
		 * the second on the real number line.
		 */
		bool Split(double alpha, Interval& intervalA, Interval& intervalB) const;

		/**
		 * Pick a random value within this interval and return it.
		 */
		double Random() const;

		/**
		 * Write this interval to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this interval from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		double A, B;
	};

	inline bool operator<(const Interval& intervalA, const Interval& intervalB)
	{
		return intervalA.B < intervalB.A;
	}

	inline bool operator>(const Interval& intervalA, const Interval& intervalB)
	{
		return intervalA.A > intervalB.B;
	}

	inline bool operator<(const Interval& interval, double value)
	{
		return interval.B < value;
	}

	inline bool operator>(const Interval& interval, double value)
	{
		return interval.A > value;
	}

	inline bool operator<(double value, const Interval& interval)
	{
		return value < interval.A;
	}

	inline bool operator>(double value, const Interval& interval)
	{
		return value > interval.B;
	}

	inline bool operator<=(const Interval& intervalA, const Interval& intervalB)
	{
		return intervalA.B <= intervalB.A;
	}

	inline bool operator>=(const Interval& intervalA, const Interval& intervalB)
	{
		return intervalA.A >= intervalB.B;
	}

	inline bool operator<=(const Interval& interval, double value)
	{
		return interval.B <= value;
	}

	inline bool operator>=(const Interval& interval, double value)
	{
		return interval.A >= value;
	}

	inline bool operator<=(double value, const Interval& interval)
	{
		return value <= interval.A;
	}

	inline bool operator>=(double value, const Interval& interval)
	{
		return value >= interval.B;
	}
}