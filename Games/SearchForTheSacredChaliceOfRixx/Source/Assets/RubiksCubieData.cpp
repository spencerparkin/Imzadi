#include "RubiksCubieData.h"
#include "Log.h"

RubiksCubieData::RubiksCubieData()
{
}

/*virtual*/ RubiksCubieData::~RubiksCubieData()
{
}

/*virtual*/ bool RubiksCubieData::Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache)
{
	if (!MovingPlatformData::Load(jsonDoc, assetCache))
		return false;

	if (!jsonDoc.HasMember("cubie_to_puzzle") || !LoadTransform(jsonDoc["cubie_to_puzzle"], this->cubieToPuzzle))
	{
		IMZADI_LOG_ERROR("Failed to load \"cubie_to_puzzle\" transform member.");
		return false;
	}

	if (!jsonDoc.HasMember("master") || !jsonDoc["master"].IsString())
	{
		IMZADI_LOG_ERROR("No \"master\" member found or it's not a string.");
		return false;
	}

	if (!jsonDoc.HasMember("puzzle_channel") || !jsonDoc["puzzle_channel"].IsString())
	{
		IMZADI_LOG_ERROR("No \"puzzle_channel\" member found or it's not a string.");
		return false;
	}

	this->masterName = jsonDoc["master"].GetString();
	this->puzzleChannelName = jsonDoc["puzzle_channel"].GetString();

	return true;
}

/*virtual*/ bool RubiksCubieData::Unload()
{
	MovingPlatformData::Unload();

	return true;
}