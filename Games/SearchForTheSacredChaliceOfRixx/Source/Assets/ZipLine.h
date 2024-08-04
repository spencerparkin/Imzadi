#pragma once

#include "AssetCache.h"
#include "Math/LineSegment.h"

class ZipLine : public Imzadi::Asset
{
public:
	ZipLine();
	virtual ~ZipLine();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Unload() override;

private:
	Imzadi::LineSegment lineSegment;	///< This is the zip-line geometry.
	double radius;						///< We imagine a sphere with this radius at each end of the line-segment.
};