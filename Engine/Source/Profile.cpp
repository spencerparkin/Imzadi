#include "Profile.h"
#include <algorithm>
#include <format>

using namespace Imzadi;

ProfileBlock::ProfileBlock(ProfileData* data, const std::string& name)
{
	this->data = data;
	this->name = name;
	this->clock.Reset();
}

/*virtual*/ ProfileBlock::~ProfileBlock()
{
	double timeSpentMS = this->clock.GetCurrentTimeMilliseconds();
	this->data->AccumulateInfo(this->name, timeSpentMS);
}

ProfileData::ProfileData()
{
}

/*virtual*/ ProfileData::~ProfileData()
{
}

void ProfileData::AccumulateInfo(const std::string& blockName, double timeSpentMS)
{
	BlockMap::iterator iter = this->blockMap.find(blockName);
	if (iter != this->blockMap.end())
	{
		Block& block = iter->second;
		block.hitCount++;
		block.totalTimeMS += timeSpentMS;
	}
	else
	{
		Block block;
		block.name = blockName;
		block.hitCount = 1;
		block.totalTimeMS = timeSpentMS;
		this->blockMap.insert(std::pair<std::string, Block>(blockName, block));
	}
}

void ProfileData::Reset()
{
	this->blockMap.clear();
}

std::string ProfileData::PrintStats() const
{
	std::vector<Block> blockArray;
	for (auto pair : this->blockMap)
		blockArray.push_back(pair.second);

	std::sort(blockArray.begin(), blockArray.end(), [](const Block& blockA, const Block& blockB) -> bool
		{
			return blockA.totalTimeMS < blockB.totalTimeMS;
		});

	std::string stats;
	stats += "======================================\n";
	stats += std::format("Num. profile blocks: {}\n", blockArray.size());

	for (const Block& block : blockArray)
	{
		stats += "---------------------------------\n";
		stats += std::format("Name: {}\n", block.name.c_str());
		stats += std::format("Hit Count: {}\n", block.hitCount);
		stats += std::format("Total Time MS: {}\n", block.totalTimeMS);
	}

	return stats;
}