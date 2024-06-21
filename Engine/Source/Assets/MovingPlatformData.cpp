#include "MovingPlatformData.h"
#include "Log.h"

using namespace Imzadi;

MovingPlatformData::MovingPlatformData()
{
	this->moveSpeed = 0.0;
	this->splineDeltas = new std::vector<Vector3>();
}

/*virtual*/ MovingPlatformData::~MovingPlatformData()
{
	delete this->splineDeltas;
}

/*virtual*/ bool MovingPlatformData::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.HasMember("mesh") || !jsonDoc["mesh"].IsString())
	{
		IMZADI_LOG_ERROR("No \"mesh\" member found or it's not a string.");
		return false;
	}

	this->meshFile = jsonDoc["mesh"].GetString();

	if (!jsonDoc.HasMember("collision") || !jsonDoc["collision"].IsString())
	{
		IMZADI_LOG_ERROR("No \"collision\" member found or it's not a string.");
		return false;
	}

	this->collisionFile = jsonDoc["collision"].GetString();

	if (!jsonDoc.HasMember("spline_deltas") || !jsonDoc["spline_deltas"].IsArray())
	{
		IMZADI_LOG_ERROR("No \"spline_deltas\" member found or it's not an array.");
		return false;
	}

	this->splineDeltas->clear();
	for (int i = 0; i < jsonDoc["spline_deltas"].Size(); i++)
	{
		Vector3 delta;
		if (!this->LoadVector(jsonDoc["spline_deltas"][i], delta))
		{
			IMZADI_LOG_ERROR(std::format("Failed to load spline delta {}.", i));
			return false;
		}

		this->splineDeltas->push_back(delta);
	}

	if (!jsonDoc.HasMember("spline_type") || !jsonDoc["spline_type"].IsString())
	{
		IMZADI_LOG_ERROR("No \"spline_type\" member found or it's not a string.");
		return false;
	}

	std::string splineTypeStr = jsonDoc["spline_type"].GetString();
	if (splineTypeStr == "lerp")
		this->splineType = SplineType::LERP;
	else if (splineTypeStr == "smooth")
		this->splineType = SplineType::SMOOTH;
	else
	{
		IMZADI_LOG_ERROR(std::format("Could not decypher \"{}\" as a spline-type.", splineTypeStr.c_str()));
		return false;
	}

	if (!jsonDoc.HasMember("spline_mode") || !jsonDoc["spline_mode"].IsString())
	{
		IMZADI_LOG_ERROR("No \"spline_mode\" given or it's not a string.");
		return false;
	}

	std::string splineModeStr = jsonDoc["spline_mode"].GetString();
	if (splineModeStr == "bounce")
		this->splineMode = SplineMode::BOUNCE;
	else if (splineModeStr == "cycle")
		this->splineMode = SplineMode::CYCLE;
	else
	{
		IMZADI_LOG_ERROR(std::format("Could not decypher \"{}\" as a spline mode.", splineModeStr.c_str()));
		return false;
	}

	if (!jsonDoc.HasMember("move_speed") || !jsonDoc["move_speed"].IsFloat())
	{
		IMZADI_LOG_ERROR("No \"move_speed\" given or it's not a float.");
		return false;
	}

	this->moveSpeed = jsonDoc["move_speed"].GetFloat();

	return true;
}

/*virtual*/ bool MovingPlatformData::Unload()
{
	return true;
}