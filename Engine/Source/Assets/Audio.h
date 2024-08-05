#pragma once

#include "AssetCache.h"
#include "FileDatas/AudioData.h"
#include "FileFormat.h"

namespace Imzadi
{
	/**
	 * These are chunks of audio meant as one-shot sound-fx or ambient sound meant to play on a loop,
	 * or even music or whatever.
	 */
	class IMZADI_API Audio : public Asset
	{
	public:
		Audio();
		virtual ~Audio();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		const std::string& GetName() const { return this->name; }

	protected:
		AudioDataLib::AudioData* audioData;
		bool looped;
		std::string name;
	};
}