#include "Audio.h"
#include "Log.h"
#include "ByteStream.h"
#include "Error.h"
#include <filesystem>

using namespace Imzadi;

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
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected JSON object at root of JSON audio asset data.");
		return false;
	}

	if (!jsonDoc.HasMember("audio_data_file") || !jsonDoc["audio_data_file"].IsString())
	{
		IMZADI_LOG_ERROR("Did not find \"audio_data_file\" member, or it wasn't a string.");
		return false;
	}

	std::string audioDataFile = jsonDoc["audio_data_file"].GetString();
	if (!assetCache->ResolveAssetPath(audioDataFile))
	{
		IMZADI_LOG_ERROR("Failed to resolve path to audio data file: %s", audioDataFile.c_str());
		return false;
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
	AudioDataLib::FileData* fileData = nullptr;
	if (!fileFormat->ReadFromStream(fileStream, fileData, error))
	{
		IMZADI_LOG_ERROR("Failed to parse audio file \"%s\" with error: %s", audioDataFile.c_str(), error.GetErrorMessage().c_str());
		return false;
	}

	this->audioData = dynamic_cast<AudioDataLib::AudioData*>(fileData);
	if (!this->audioData)
	{
		IMZADI_LOG_ERROR("Didn't load audio data from file \"%s\".  Loaded something else?!", audioDataFile.c_str());
		delete fileData;
		return false;
	}

	if (jsonDoc.HasMember("looped") && jsonDoc["looped"].IsBool())
		this->looped = jsonDoc["looped"].GetBool();
	else
		this->looped = false;

	if (jsonDoc.HasMember("name") && jsonDoc["name"].IsString())
		this->name = jsonDoc["name"].GetString();
	else
	{
		std::filesystem::path path(audioDataFile);
		this->name = path.stem().string();
	}

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
	if (this->audioData)
	{
		AudioDataLib::AudioData::Destroy(this->audioData);
		this->audioData = nullptr;
	}

	this->name = "";

	return true;
}