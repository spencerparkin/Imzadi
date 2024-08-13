#pragma once

#include "Defines.h"

namespace Imzadi
{
	/**
	 * Instances of this class can be used to measure time as accurately as possible.
	 */
	class IMZADI_API Clock
	{
	public:
		Clock();
		virtual ~Clock();

		bool NeverBeenReset() const;
		void Reset();

		double GetCurrentTimeMilliseconds() const;
		double GetCurrentTimeSeconds() const;

	private:
		uint64_t GetCurrentSystemTime() const;
		uint64_t GetElapsedTime() const;

		uint64_t timeBase;
	};
}