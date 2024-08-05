#include "ZipLine.h"

ZipLine::ZipLine()
{
	this->radius = 0.0;
	this->frictionFactor = 0.5;
}

/*virtual*/ ZipLine::~ZipLine()
{
}

/*virtual*/ bool ZipLine::Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache)
{
	if (!jsonDoc.HasMember("line_segment"))
		return false;

	const rapidjson::Value& lineSegmentValue = jsonDoc["line_segment"];
	if (!lineSegmentValue.IsObject())
		return false;

	if (!LoadLineSegment(lineSegmentValue, this->lineSegment))
		return false;

	if (!jsonDoc.HasMember("radius") || !jsonDoc["radius"].IsFloat())
		return false;

	this->radius = jsonDoc["radius"].GetFloat();

	if (jsonDoc.HasMember("friction_factor") && jsonDoc["friction_factor"].IsFloat())
		this->frictionFactor = jsonDoc["friction_factor"].GetFloat();

	if (this->lineSegment.Length() == 0.0)
		return false;

	if (this->lineSegment.point[0].y <= this->lineSegment.point[1].y)
		return false;

	if (this->radius <= 0.0)
		return false;

	if (this->frictionFactor < 0.0)
		return false;

	return true;
}

/*virtual*/ bool ZipLine::Unload()
{
	return true;
}