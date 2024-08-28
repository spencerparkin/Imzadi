#pragma once

#include "AssetCache.h"
#include "FileDatas/AudioData.h"
#include "FileDatas/MidiData.h"
#include "FileFormat.h"
#include <xaudio2.h>

namespace Imzadi
{
	/**
	 * These are any kind of asset loaded by the audio sub-system.
	 */
	class IMZADI_API AudioSystemAsset : public Asset
	{
	public:
		AudioSystemAsset();
		virtual ~AudioSystemAsset();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		const std::string& GetName() const { return this->name; }

	protected:
		bool LoadAudioFileData(const rapidjson::Document& jsonDoc, AssetCache* assetCache, const std::string& fileKey, AudioDataLib::FileData*& fileData);

		std::string name;
	};

	/**
	 * These are chunks of audio meant as one-shot sound-fx or ambient sound meant to play on a loop,
	 * or even music or whatever.
	 */
	class IMZADI_API Audio : public AudioSystemAsset
	{
	public:
		Audio();
		virtual ~Audio();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;
		
		const WAVEFORMATEX& GetWaveFormat() const { return this->waveFormat; }
		const AudioDataLib::AudioData* GetAudioData() const { return this->audioData; }

	protected:
		AudioDataLib::AudioData* audioData;
		bool looped;
		WAVEFORMATEX waveFormat;
	};

	/**
	 * These are MIDI files that can be played back in the audio sub-system using a MIDI synthesizer device.
	 */
	class IMZADI_API MidiSong : public AudioSystemAsset
	{
	public:
		MidiSong();
		virtual ~MidiSong();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		AudioDataLib::MidiData* GetMidiData() { return this->midiData; }

	protected:
		AudioDataLib::MidiData* midiData;
	};
}