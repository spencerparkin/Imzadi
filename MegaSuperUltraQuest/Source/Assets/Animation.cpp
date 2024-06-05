#include "Animation.h"

//------------------------------- Animation -------------------------------

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

bool Animation::FindKeyFrames(const KeyFrame*& keyFrameA, const KeyFrame*& keyFrameB, double timeIndex, double& alpha) const
{
	return false;
}

//------------------------------- KeyFrame -------------------------------

KeyFrame::KeyFrame()
{
}

/*virtual*/ KeyFrame::~KeyFrame()
{
}