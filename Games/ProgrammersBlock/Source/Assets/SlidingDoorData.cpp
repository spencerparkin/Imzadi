#include "SlidingDoorData.h"

SlidingDoorData::SlidingDoorData()
{
	this->initiallyLocked = false;
}

/*virtual*/ SlidingDoorData::~SlidingDoorData()
{
}

/*virtual*/ bool SlidingDoorData::Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache)
{
	if (!MovingPlatformData::Load(jsonDoc, assetCache))
		return false;

	if (jsonDoc.HasMember("initially_locked") && jsonDoc["initially_locked"].IsBool())
		this->initiallyLocked = jsonDoc["initially_locked"].GetBool();

	if (jsonDoc.HasMember("door_channel") && jsonDoc["door_channel"].IsString())
		this->doorChannel = jsonDoc["door_channel"].GetString();

	if (jsonDoc.HasMember("trigger_box_remove") && jsonDoc["trigger_box_remove"].IsString())
		this->triggerBoxRemove = jsonDoc["trigger_box_remove"].GetString();

	return true;
}

/*virtual*/ bool SlidingDoorData::Unload()
{
	MovingPlatformData::Unload();

	return true;
}