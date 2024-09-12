#pragma once

#include "Assets/MovingPlatformData.h"

/**
 * This is the data used to configure a sliding door entity.
 */
class SlidingDoorData : public Imzadi::MovingPlatformData
{
public:
	SlidingDoorData();
	virtual ~SlidingDoorData();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Unload() override;

	bool IsInitiallyLocked() const { return this->initiallyLocked; }
	const std::string& GetDoorChannel() const { return this->doorChannel; }

private:
	bool initiallyLocked;
	std::string doorChannel;
};