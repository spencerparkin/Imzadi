#pragma once

#include "AssetCache.h"

class Skeleton;

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
	 * Use this animation and the given time index to pose the given skeleton.
	 */
	void PoseSkeleton(Skeleton* skeleton, double timeSeconds) const;

private:
	// TODO: Add members here.
};