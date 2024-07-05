#include "MovingPlatformData.h"
#include "Log.h"
#include "Math/Angle.h"

using namespace Imzadi;

MovingPlatformData::MovingPlatformData()
{
	this->moveSpeedUnitsPerSecond = 0.0;
	this->rotationSpeedDegreesPerSecond = 0.0;
	this->splineDeltas = new std::vector<DeltaInfo>();
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
		const rapidjson::Value& splineDeltaValue = jsonDoc["spline_deltas"][i];

		Vector3 translation;
		if (!this->LoadVector(splineDeltaValue, translation))
			translation.SetComponents(0.0, 0.0, 0.0);

		Quaternion rotation;
		if (!this->LoadEulerAngles(splineDeltaValue, rotation))
			rotation.SetIdentity();

		DeltaInfo deltaInfo;
		deltaInfo.transform.translation = translation;
		deltaInfo.transform.matrix.SetFromQuat(rotation);
		deltaInfo.lingerTimeSeconds = 0.0;

		if (splineDeltaValue.HasMember("linger") && splineDeltaValue["linger"].IsFloat())
			deltaInfo.lingerTimeSeconds = splineDeltaValue["linger"].GetFloat();

		this->splineDeltas->push_back(deltaInfo);
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

	this->moveSpeedUnitsPerSecond = 0.0;
	if (jsonDoc.HasMember("move_speed") && jsonDoc["move_speed"].IsFloat())
		this->moveSpeedUnitsPerSecond = jsonDoc["move_speed"].GetFloat();

	this->rotationSpeedDegreesPerSecond = 0.0;
	if (jsonDoc.HasMember("rotation_speed") && jsonDoc["rotation_speed"].IsFloat())
		this->rotationSpeedDegreesPerSecond = Angle::DegreesToRadians(jsonDoc["rotation_speed"].GetFloat());

	return true;
}

/*virtual*/ bool MovingPlatformData::Unload()
{
	return true;
}