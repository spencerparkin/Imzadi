#include "Animation.h"

Animation::Animation()
{
}

/*virtual*/ Animation::~Animation()
{
}

/*virtual*/ bool Animation::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	return false;
}

/*virtual*/ bool Animation::Unload()
{
	return false;
}

void Animation::PoseSkeleton(Skeleton* skeleton, double timeSeconds) const
{
	// TODO: Write this.
}