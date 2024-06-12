#include "MovingPlatformData.h"

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

/*virtual*/ bool MovingPlatformData::Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache)
{
	if (!jsonDoc.HasMember("mesh") || !jsonDoc["mesh"].IsString())
		return false;

	this->meshFile = jsonDoc["mesh"].GetString();

	if (!jsonDoc.HasMember("collision") || !jsonDoc["collision"].IsString())
		return false;

	this->collisionFile = jsonDoc["collision"].GetString();

	if (!jsonDoc.HasMember("spline_deltas") || !jsonDoc["spline_deltas"].IsArray())
		return false;

	this->splineDeltas->clear();
	for (int i = 0; i < jsonDoc["spline_deltas"].Size(); i++)
	{
		Vector3 delta;
		if (!this->LoadVector(jsonDoc["spline_deltas"][i], delta))
			return false;

		this->splineDeltas->push_back(delta);
	}

	if (!jsonDoc.HasMember("spline_type") || !jsonDoc["spline_type"].IsString())
		return false;

	std::string splineTypeStr = jsonDoc["spline_type"].GetString();
	if (splineTypeStr == "lerp")
		this->splineType = SplineType::LERP;
	else if (splineTypeStr == "smooth")
		this->splineType = SplineType::SMOOTH;
	else
		return false;

	if (!jsonDoc.HasMember("spline_mode") || !jsonDoc["spline_mode"].IsString())
		return false;

	std::string splineModeStr = jsonDoc["spline_mode"].GetString();
	if (splineModeStr == "bounce")
		this->splineMode = SplineMode::BOUNCE;
	else if (splineModeStr == "cycle")
		this->splineMode = SplineMode::CYCLE;
	else
		return false;

	if (!jsonDoc.HasMember("move_speed") || !jsonDoc["move_speed"].IsFloat())
		return false;

	this->moveSpeed = jsonDoc["move_speed"].GetFloat();

	return true;
}

/*virtual*/ bool MovingPlatformData::Unload()
{
	return true;
}