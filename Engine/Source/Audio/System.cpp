#include "System.h"
#include "Log.h"
#include "Game.h"
#include "AssetCache.h"

using namespace Imzadi;

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

	return true;
}

bool AudioSystem::Shutdown()
{
	this->audioMap.clear();

	// TODO: Delete source voices here before submixes.
	// TODO: Delete submix voices here before master.

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

bool AudioSystem::PlayAmbientSounds(const std::set<std::string>& ambientSoundsSet)
{
	// TODO: Write this.
	return false;
}

bool AudioSystem::PlayAmbientSoundOccationally(const std::string& ambientSound, double minFrequency, double maxFrequency)
{
	// TODO: Write this.
	return false;
}

bool AudioSystem::PlaySound(const std::string& sound)
{
	// TODO: Write this.
	return false;
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