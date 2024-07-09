#include "TriggerBoxData.h"
#include "Log.h"

using namespace Imzadi;

TriggerBoxData::TriggerBoxData()
{
	this->eventChannelName = "TriggerBox";
}

/*virtual*/ TriggerBoxData::~TriggerBoxData()
{
}

/*virtual*/ bool TriggerBoxData::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected trigger box data to be a JSON object.");
		return false;
	}

	if (!jsonDoc.HasMember("box"))
	{
		IMZADI_LOG_ERROR("Expected trigger box data to have \"box\" member in the JSON data.");
		return false;
	}

	if (!Asset::LoadBoundingBox(jsonDoc["box"], this->box))
	{
		IMZADI_LOG_ERROR("Failed to load bounding box!");
		return false;
	}

	if (!jsonDoc.HasMember("name") || !jsonDoc["name"].IsString())
	{
		IMZADI_LOG_ERROR("No \"name\" field given or it's not a string.");
		return false;
	}

	this->name = jsonDoc["name"].GetString();

	if (jsonDoc.HasMember("event_channel_name") && jsonDoc["event_channel_name"].IsString())
		this->eventChannelName = jsonDoc["event_channel_name"].GetString();

	return true;
}

/*virtual*/ bool TriggerBoxData::Unload()
{
	return true;
}