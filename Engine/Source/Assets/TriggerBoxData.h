#pragma once

#include "AssetCache.h"
#include "Math/AxisAlignedBoundingBox.h"

namespace Imzadi
{
	/**
	 * This is data the engine can load for trigger boxes.
	 */
	class IMZADI_API TriggerBoxData : public Asset
	{
	public:
		TriggerBoxData();
		virtual ~TriggerBoxData();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		const AxisAlignedBoundingBox& GetBox() const { return this->box; }
		const std::string& GetName() const { return this->name; }

	private:
		AxisAlignedBoundingBox box;
		std::string name;
	};
}