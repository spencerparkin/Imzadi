#include "SkinWeights.h"

SkinWeights::SkinWeights()
{
}

/*virtual*/ SkinWeights::~SkinWeights()
{
}

/*virtual*/ bool SkinWeights::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	return false;
}

/*virtual*/ bool SkinWeights::Unload()
{
	return true;
}

/*virtual*/ bool SkinWeights::Save(rapidjson::Document& jsonDoc) const
{
	return false;
}