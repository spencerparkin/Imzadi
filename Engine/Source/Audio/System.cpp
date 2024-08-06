#include "System.h"
#include "Log.h"
#include "Game.h"
#include "AssetCache.h"

using namespace Imzadi;

//------------------------------------- AudioSystem -------------------------------------

AudioSystem::AudioSystem()
{
	this->audio = nullptr;
	this->masteringVoice = nullptr;
}

/*virtual*/ AudioSystem::~AudioSystem()
{
}

bool AudioSystem::Initialize()
{
	if (this->audio)
		return false;

	HRESULT result = XAudio2Create(&this->audio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create XAudio2 interface.  Error code: %x", result);
		return false;
	}

	result = this->audio->CreateMasteringVoice(&this->masteringVoice);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create mastering voice.  Error code: %x", result);
		return false;
	}

#if defined _DEBUG
	XAUDIO2_DEBUG_CONFIGURATION debugConfig{};
	debugConfig.TraceMask = XAUDIO2_LOG_ERRORS;
	debugConfig.BreakMask = XAUDIO2_LOG_ERRORS;
	debugConfig.LogFunctionName = TRUE;
	this->audio->SetDebugConfiguration(&debugConfig);
#endif _DEBUG

	return true;
}

bool AudioSystem::Shutdown()
{
	this->ClearAllAmbientSounds();
	this->ClearSourceCache();
	this->audioMap.clear();

	if (this->masteringVoice)
	{
		this->masteringVoice->DestroyVoice();
		this->masteringVoice = nullptr;
	}

	if (this->audio)
	{
		this->audio->Release();
		this->audio = nullptr;
	}

	return true;
}

bool AudioSystem::AddAmbientSound(const std::set<std::string>& soundSet, const Interval& delayRange, bool startNow, float volume)
{
	if (!delayRange.IsValid())
		return false;

	if (soundSet.size() == 0)
		return false;

	for (const std::string& sound : soundSet)
		if (this->audioMap.find(sound) == this->audioMap.end())
			return false;

	AmbientSound ambientSound;
	ambientSound.soundSet = soundSet;
	ambientSound.delayRangeSeconds = delayRange;
	ambientSound.waitTimeSeconds = startNow ? 0.0 : delayRange.Random();
	ambientSound.elapsedTimeSeconds = 0.0;
	ambientSound.volume = volume;

	this->ambientSoundArray.push_back(ambientSound);
	return true;
}

void AudioSystem::ClearAllAmbientSounds()
{
	this->ambientSoundArray.clear();
}

void AudioSystem::Tick(double deltaTimeSeconds)
{
	for (AmbientSound& ambientSound : this->ambientSoundArray)
	{
		ambientSound.elapsedTimeSeconds += deltaTimeSeconds;

		if (ambientSound.elapsedTimeSeconds >= ambientSound.waitTimeSeconds)
		{
			ambientSound.elapsedTimeSeconds -= ambientSound.waitTimeSeconds;
			ambientSound.waitTimeSeconds = ambientSound.delayRangeSeconds.Random();

			int j = (int)::round(Interval(0.0, double(ambientSound.soundSet.size() - 1)).Random());
			std::set<std::string>::iterator iter = ambientSound.soundSet.begin();
			for (int i = 0; i < j; i++)
				iter++;

			const std::string& sound = *iter;
			this->PlaySound(sound, ambientSound.volume);
		}
	}
}

bool AudioSystem::PlaySound(const std::string& sound, float volume /*= 1.0f*/)
{
	AudioMap::iterator iter = this->audioMap.find(sound);
	if (iter == this->audioMap.end())
	{
		IMZADI_LOG_ERROR("Did not find sound with name \"%s\".", sound.c_str());
		return false;
	}

	const Audio* audioAsset = iter->second.Get();

	AudioSourceList* audioSourceList = this->GetOrCreateAudioSourceList(audioAsset->GetWaveFormat());
	if (!audioSourceList)
	{
		IMZADI_LOG_ERROR("Failed to get or create audio source list.");
		return false;
	}

	AudioSource* audioSource = audioSourceList->GetOrCreateUnusedAudioSource(audioAsset->GetWaveFormat());
	if (!audioSource)
	{
		IMZADI_LOG_ERROR("Failed to get or create audio source.");
		return false;
	}

	const AudioDataLib::AudioData* audioData = audioAsset->GetAudioData();

	XAUDIO2_BUFFER audioBuffer{};
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.AudioBytes = audioData->GetAudioBufferSize();
	audioBuffer.pAudioData = audioData->GetAudioBuffer();

	audioSource->inUse = true;

	HRESULT result = audioSource->sourceVoice->SubmitSourceBuffer(&audioBuffer);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to submit audio buffer to source voice.");
		audioSource->inUse = false;
		return false;
	}

	audioSource->sourceVoice->SetVolume(volume);
	audioSource->sourceVoice->Start();

	return true;
}

bool AudioSystem::LoadAudioDirectory(const std::string& audioDirectory, bool recursive)
{
	AssetCache* assetCache = Game::Get()->GetAssetCache();
	std::string fullyQualifiedPath = audioDirectory;
	if (!assetCache->ResolveAssetPath(fullyQualifiedPath))
		return false;

	std::filesystem::path path(fullyQualifiedPath);

	for (auto const& dirEntry : std::filesystem::directory_iterator(path))
	{
		if (recursive && dirEntry.is_directory())
		{
			if(!this->LoadAudioDirectory(dirEntry.path().string(), recursive))
				return false;
		}
		else
		{
			std::string assetFile = dirEntry.path().string();
			std::string ext = std::filesystem::path(assetFile).extension().string();
			if (ext == ".audio")
			{
				Reference<Asset> asset;
				if (!assetCache->LoadAsset(assetFile, asset))
					return false;

				Reference<Audio> audioAsset;
				audioAsset.SafeSet(asset.Get());
				if (!audioAsset)
				{
					IMZADI_LOG_ERROR("Loaded something, but it wasn't audio!");
					return false;
				}

				std::string audioName = audioAsset->GetName();
				if (audioName.length() == 0)
				{
					IMZADI_LOG_ERROR("Audio asset (%s) has blank name.", assetFile.c_str());
					return false;
				}

				if (this->audioMap.find(audioName) != this->audioMap.end())
				{
					IMZADI_LOG_WARNING("Audio with name \"%s\" already loaded!  Loading over it.", audioName.c_str());
				}

				this->audioMap.insert(std::pair<std::string, Reference<Audio>>(audioName, audioAsset));
			}
		}
	}

	return true;
}

void AudioSystem::ClearSourceCache()
{
	this->audioSourceCache.clear();
}

AudioSystem::AudioSourceList* AudioSystem::GetOrCreateAudioSourceList(const WAVEFORMATEX& waveFormat)
{
	AudioSourceList* audioSourceList = nullptr;

	AudioSourceCache::iterator iter = this->audioSourceCache.find(waveFormat);
	if (iter != this->audioSourceCache.end())
		audioSourceList = iter->second.Get();
	else
	{
		audioSourceList = new AudioSourceList();
		this->audioSourceCache.insert(std::pair<WAVEFORMATEX, AudioSourceList*>(waveFormat, audioSourceList));
	}

	return audioSourceList;
}

//------------------------------------- AudioSystem::AudioSource -------------------------------------

AudioSystem::AudioSource::AudioSource()
{
	this->inUse = false;
	this->sourceVoice = nullptr;
	::memset(&this->waveFormat, 0, sizeof(this->waveFormat));
}

/*virtual*/ AudioSystem::AudioSource::~AudioSource()
{
}

/*virtual*/ void AudioSystem::AudioSource::OnVoiceProcessingPassStart(UINT32 bytesRequired)
{
}

/*virtual*/ void AudioSystem::AudioSource::OnVoiceProcessingPassEnd()
{
}

/*virtual*/ void AudioSystem::AudioSource::OnStreamEnd()
{
	this->inUse = false;
}

/*virtual*/ void AudioSystem::AudioSource::OnBufferStart(void* bufferContext)
{
}

/*virtual*/ void AudioSystem::AudioSource::OnBufferEnd(void* bufferContext)
{
}

/*virtual*/ void AudioSystem::AudioSource::OnLoopEnd(void* bufferContext)
{
}

/*virtual*/ void AudioSystem::AudioSource::OnVoiceError(void* bufferContext, HRESULT Error)
{
}

//------------------------------------- AudioSystem::AudioSourceList -------------------------------------

AudioSystem::AudioSourceList::AudioSourceList()
{
}

/*virtual*/ AudioSystem::AudioSourceList::~AudioSourceList()
{
	this->Clear();
}

void AudioSystem::AudioSourceList::Clear()
{
	for (AudioSource* audioSource : this->audioSourceList)
	{
		audioSource->sourceVoice->Stop();
		audioSource->sourceVoice->DestroyVoice();
	}

	this->audioSourceList.clear();
}

AudioSystem::AudioSource* AudioSystem::AudioSourceList::GetOrCreateUnusedAudioSource(const WAVEFORMATEX& waveFormat)
{
	for (AudioSource* audioSource : this->audioSourceList)
		if (!audioSource->inUse)
			return audioSource;

	AudioSource* audioSource = new AudioSource();
	audioSource->waveFormat = waveFormat;
	
	AudioSystem* audioSystem = Game::Get()->GetAudioSystem();

	HRESULT result = audioSystem->audio->CreateSourceVoice(&audioSource->sourceVoice, &waveFormat, 0, 2.0f, audioSource);
	if (FAILED(result))
	{
		delete audioSource;
		return nullptr;
	}

	this->audioSourceList.push_back(audioSource);

	return audioSource;
}