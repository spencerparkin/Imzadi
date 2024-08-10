#pragma once

#include "Defines.h"
#include "Clock.h"
#include <string>
#include <unordered_map>

namespace Imzadi
{
	class ProfileData;

	/**
	 * These can be used to measure the time spent in a scope block.
	 * Macros should be used rather than this class directly so that
	 * all instances of it can compile out of a release build.
	 * 
	 * Note that we don't handle recursive blocks here.
	 */
	class IMZADI_API ProfileBlock
	{
	public:
		ProfileBlock(ProfileData* data, const std::string& name);
		virtual ~ProfileBlock();

	private:
		Clock clock;
		ProfileData* data;
		std::string name;
	};

	/**
	 * An instance of this class can collect stats from a bunch of profile
	 * blocks sprinkled throughout the code.  Note that it is not thread-safe.
	 * Each thread should just use their own instance of this class.
	 */
	class IMZADI_API ProfileData
	{
	public:
		ProfileData();
		virtual ~ProfileData();

		void AccumulateInfo(const std::string& blockName, double timeSpentMS);
		void Reset();
		std::string PrintStats() const;

	private:
		struct Block
		{
			std::string name;
			uint32_t hitCount;
			double totalTimeMS;
		};

		typedef std::unordered_map<std::string, Block> BlockMap;
		BlockMap blockMap;
	};
}