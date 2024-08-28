#include "Audio.h"
#include "Log.h"
#include "ByteStream.h"
#include "Error.h"
#include <filesystem>

using namespace Imzadi;

//------------------------------------- AudioSystemAsset -------------------------------------

AudioSystemAsset::AudioSystemAsset()
{
}

/*virtual*/ AudioSystemAsset::~AudioSystemAsset()
{
}

/*virtual*/ bool AudioSystemAsset::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected JSON object at root of JSON audio system asset data.");
		return false;
	}

	if (jsonDoc.HasMember("name") && jsonDoc["name"].IsString())
		this->name = jsonDoc["name"].GetString();

	return true;
}

/*virtual*/ bool AudioSystemAsset::Unload()
{
	this->name = "";
	return true;
}

bool AudioSystemAsset::LoadAudioFileData(const rapidjson::Document& jsonDoc, AssetCache* assetCache, const std::string& fileKey, AudioDataLib::FileData*& fileData)
{
	fileData = nullptr;

	if (!jsonDoc.HasMember(fileKey.c_str()) || !jsonDoc[fileKey.c_str()].IsString())
	{
		IMZADI_LOG_ERROR("Did not find \"%s\" member, or it wasn't a string.", fileKey.c_str());
		return false;
	}

	std::string audioDataFile = jsonDoc[fileKey.c_str()].GetString();
	if (!assetCache->ResolveAssetPath(audioDataFile))
	{
		IMZADI_LOG_ERROR("Failed to resolve path to audio data file: %s", audioDataFile.c_str());
		return false;
	}

	if (this->name.length() == 0)
	{
		std::filesystem::path path(audioDataFile);
		this->name = path.stem().string();
	}

	std::shared_ptr<AudioDataLib::FileFormat> fileFormat = AudioDataLib::FileFormat::CreateForFile(audioDataFile);
	if (!fileFormat)
	{
		IMZADI_LOG_ERROR("The audio file format for file \"%s\" is not recognized or not yet supported.", audioDataFile.c_str());
		return false;
	}

	AudioDataLib::FileInputStream fileStream(audioDataFile.c_str());
	if (!fileStream.IsOpen())
	{
		IMZADI_LOG_ERROR("Failed to open file: %s", audioDataFile.c_str());
		return false;
	}

	AudioDataLib::Error error;
	if (!fileFormat->ReadFromStream(fileStream, fileData, error))
	{
		IMZADI_LOG_ERROR("Failed to parse audio file \"%s\" with error: %s", audioDataFile.c_str(), error.GetErrorMessage().c_str());
		return false;
	}

	return true;
}

//------------------------------------- Audio -------------------------------------

Audio::Audio()
{
	this->looped = false;
	this->audioData = nullptr;
	::memset(&this->waveFormat, 0, sizeof(this->waveFormat));
}

/*virtual*/ Audio::~Audio()
{
}

/*virtual*/ bool Audio::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!AudioSystemAsset::Load(jsonDoc, assetCache))
		return false;

	AudioDataLib::FileData* fileData = nullptr;
	if (!this->LoadAudioFileData(jsonDoc, assetCache, "audio_data_file", fileData))
		return false;

	this->audioData = dynamic_cast<AudioDataLib::AudioData*>(fileData);
	if (!this->audioData)
	{
		IMZADI_LOG_ERROR("Didn't load audio data from file.  Loaded something else?!");
		delete fileData;
		return false;
	}

	if (jsonDoc.HasMember("looped") && jsonDoc["looped"].IsBool())
		this->looped = jsonDoc["looped"].GetBool();
	else
		this->looped = false;

	AudioDataLib::AudioData::Format& format = this->audioData->GetFormat();
	
	switch (format.sampleType)
	{
		case AudioDataLib::AudioData::Format::FLOAT:
		{
			this->waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
			break;
		}
		case AudioDataLib::AudioData::Format::SIGNED_INTEGER:
		{
			this->waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			break;
		}
		default:
		{
			IMZADI_LOG_ERROR("Format %d not yet supported.", format.sampleType);
			return false;
		}
	}

	this->waveFormat.nChannels = format.numChannels;
	this->waveFormat.nSamplesPerSec = format.SamplesPerSecondPerChannel();
	this->waveFormat.nAvgBytesPerSec = (format.framesPerSecond * format.bitsPerSample * format.numChannels) / 8;
	this->waveFormat.nBlockAlign = format.BytesPerFrame();
	this->waveFormat.wBitsPerSample = format.bitsPerSample;
	this->waveFormat.cbSize = 0;

	return true;
}

/*virtual*/ bool Audio::Unload()
{
	AudioSystemAsset::Unload();

	if (this->audioData)
	{
		AudioDataLib::AudioData::Destroy(this->audioData);
		this->audioData = nullptr;
	}

	return true;
}

//------------------------------------- MidiSong -------------------------------------

MidiSong::MidiSong()
{
	this->midiData = nullptr;
}

/*virtual*/ MidiSong::~MidiSong()
{
}

/*virtual*/ bool MidiSong::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!AudioSystemAsset::Load(jsonDoc, assetCache))
		return false;

	AudioDataLib::FileData* fileData = nullptr;
	if (!this->LoadAudioFileData(jsonDoc, assetCache, "midi_file", fileData))
		return false;

	this->midiData = dynamic_cast<AudioDataLib::MidiData*>(fileData);
	if (!this->midiData)
	{
		IMZADI_LOG_ERROR("Didn't load MIDI data from file.  Loaded something else?!");
		delete fileData;
		return false;
	}

	return true;
}

/*virtual*/ bool MidiSong::Unload()
{
	AudioSystemAsset::Unload();

	if (this->midiData)
	{
		AudioDataLib::MidiData::Destroy(this->midiData);
		this->midiData = nullptr;
	}

	return true;
}