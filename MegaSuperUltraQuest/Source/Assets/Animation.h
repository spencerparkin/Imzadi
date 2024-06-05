#pragma once

#include "AssetCache.h"

class Skeleton;
class KeyFrame;

/**
 * An animation is a sequence of key-frames.  Each key-frame specifies a moment in
 * time along with a set of transforms.  Each transform points to a named bone.
 * An animation is used to drive a skeleton (which, in turn, drives a mesh.)
 */
class Animation : public Asset
{
public:
	Animation();
	virtual ~Animation();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;

	/**
	 * Use a binary search to quickly find the adjacent pair of key-frames that are
	 * bounding (as tightly as possible) the given time index.  The caller can then
	 * pose a skeleton by interpolating between these two key-frames.
	 * 
	 * @param[out] keyFrameA This is the lower-bound key-frame.
	 * @param[out] keyFrameB This is the upper-bound key-frame.
	 * @param[in] timeSeconds This is the time (in seconds) along this animation's time-line of where to search.
	 * @param[out] alpha This is the interpolation value that can be used to blend between the two key-frames based on the given time.
	 * @return False is returned if the given time-index is out of bounds for the entire animation; true, otherwise.
	 */
	bool FindKeyFrames(const KeyFrame*& keyFrameA, const KeyFrame*& keyFrameB, double timeSeconds, double& alpha) const;

private:
	// TODO: Add members here.
};

/**
 * 
 */
class KeyFrame
{
public:
	KeyFrame();
	virtual ~KeyFrame();
};