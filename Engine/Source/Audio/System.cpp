#include "System.h"
#include "Log.h"
#include "Game.h"
#include "AssetCache.h"
#include "MidiPlayer.h"
#include "MidiMsgDestination.h"
#include "Timer.h"
#include "Error.h"

using namespace Imzadi;

//------------------------------------- AudioSystem -------------------------------------

AudioSystem::AudioSystem()
{
	this->audio = nullptr;
	this->masteringVoice = nullptr;
	this->midiThread = nullptr;
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

	this->midiThread = new MidiThread();
	if (!this->midiThread->Startup())
	{
		delete this->midiThread;
		this->midiThread = nullptr;
		IMZADI_LOG_ERROR("Failed to start MIDI thread.");
		return false;
	}

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

	if (this->midiThread)
	{
		this->midiThread->Shutdown();
		delete this->midiThread;
		this->midiThread = nullptr;
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

	const AudioSystemAsset* audioSystemAsset = iter->second.Get();

	auto audioAsset = dynamic_cast<const Audio*>(audioSystemAsset);
	if (audioAsset)
	{
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

	auto midiAsset = dynamic_cast<const MidiSong*>(audioSystemAsset);
	if (midiAsset && this->midiThread)
	{
		this->midiThread->EnqueueMidiSong(const_cast<MidiSong*>(midiAsset));
		return true;
	}

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
			if (ext == ".audio" || ext == ".song")
			{
				Reference<Asset> asset;
				if (!assetCache->LoadAsset(assetFile, asset))
					return false;

				Reference<AudioSystemAsset> audioSystemAsset;
				audioSystemAsset.SafeSet(asset.Get());
				if (!audioSystemAsset)
				{
					IMZADI_LOG_ERROR("Loaded something, but it wasn't audio!");
					return false;
				}

				std::string audioName = audioSystemAsset->GetName();
				if (audioName.length() == 0)
				{
					IMZADI_LOG_ERROR("Audio asset (%s) has blank name.", assetFile.c_str());
					return false;
				}

				if (this->audioMap.find(audioName) != this->audioMap.end())
				{
					IMZADI_LOG_WARNING("Audio with name \"%s\" already loaded!  Loading over it.", audioName.c_str());
				}

				this->audioMap.insert(std::pair<std::string, Reference<AudioSystemAsset>>(audioName, audioSystemAsset));
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

bool AudioSystem::IsMidiSongPlaying()
{
	if (!this->midiThread)
		return false;

	return this->midiThread->playing;
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

//--------------------------------------- AudioSystem::MidiThread ---------------------------------------

AudioSystem::MidiThread::MidiThread() : midiSongQueueSemaphore(0)
{
	this->thread = nullptr;
	this->exitSignaled = false;
	this->midiOut = nullptr;
	this->playing = false;
}

/*virtual*/ AudioSystem::MidiThread::~MidiThread()
{
}

bool AudioSystem::MidiThread::Startup()
{
	if (this->thread)
		return false;

	try
	{
		this->midiOut = new RtMidiOut();

		unsigned int numPorts = this->midiOut->getPortCount();
		if (numPorts == 0)
		{
			IMZADI_LOG_ERROR("No MIDI ports found!");
			return false;
		}

		for (unsigned int i = 0; i < numPorts; i++)
		{
			IMZADI_LOG_INFO("MIDI port %d is \"%s\".", i, this->midiOut->getPortName(i).c_str());
		}

		// TODO: Intelligently select MIDI port?
		this->midiOut->openPort(0);
	}
	catch (RtMidiError& error)
	{
		IMZADI_LOG_ERROR("RtMidi error: %s", error.getMessage().c_str());
	}

	if (!this->midiOut->isPortOpen())
	{
		delete this->midiOut;
		this->midiOut = nullptr;
		return false;
	}

	this->exitSignaled = false;
	this->thread = new std::thread(&MidiThread::ThreadEntryProc, this);
	return true;
}

bool AudioSystem::MidiThread::Shutdown()
{
	if (this->thread)
	{
		this->EnqueueMidiSong(nullptr);
		this->exitSignaled = true;
		this->thread->join();
		delete this->thread;
		this->thread = nullptr;
	}

	if (this->midiOut)
	{
		if (this->midiOut->isPortOpen())
			this->midiOut->closePort();
		delete this->midiOut;
		this->midiOut = nullptr;
	}

	return true;
}

void AudioSystem::MidiThread::EnqueueMidiSong(MidiSong* midiSong)
{
	std::scoped_lock<std::mutex> lock(this->midiSongQueueMutex);
	this->midiSongQueue.push_back(midiSong);
	this->midiSongQueueSemaphore.release();
}

/*static*/ void AudioSystem::MidiThread::ThreadEntryProc(MidiThread* midiThread)
{
	midiThread->Run();
}

void AudioSystem::MidiThread::Run()
{
	SetThreadDescription(GetCurrentThread(), L"MIDI Playback");

	while (true)
	{
		// Let the thread sleep (and not waste any CPU-cycles) until we have something to do.
		this->midiSongQueueSemaphore.acquire();

		// Something is wrong if we don't have anything to do when we're woken up.
		if (this->midiSongQueue.size() == 0)
			break;

		// Pull a song off our queue.
		Reference<MidiSong> midiSong;
		{
			std::scoped_lock<std::mutex> lock(this->midiSongQueueMutex);
			midiSong = *this->midiSongQueue.begin();
			this->midiSongQueue.pop_front();
		}

		// Enqueueing a null song signals us to exit.
		if (!midiSong.Get())
			break;

		this->playing = true;
		this->PlayMidiSong(midiSong.Get());
		this->playing = false;
	}

	// Clear the song queue in a thread-safe manner.
	{
		std::scoped_lock<std::mutex> lock(this->midiSongQueueMutex);
		this->midiSongQueue.clear();
	}
}

void AudioSystem::MidiThread::PlayMidiSong(MidiSong* midiSong)
{
	AudioDataLib::SystemClockTimer timer;
	AudioDataLib::MidiPlayer player(&timer);

	class MidiMsgDest : public AudioDataLib::MidiMsgDestination
	{
	public:
		MidiMsgDest(RtMidiOut* midiOut)
		{
			this->midiOut = midiOut;
		}

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, AudioDataLib::Error& error) override
		{
			try
			{
				this->midiOut->sendMessage(message, messageSize);
			}
			catch (RtMidiError& midiError)
			{
				error.Add(midiError.getMessage());
				return false;
			}

			return true;
		}

		RtMidiOut* midiOut;
	};

	std::shared_ptr<MidiMsgDest> dest(new MidiMsgDest(this->midiOut));
	player.AddDestination(dest);
	player.SetMidiData(midiSong->GetMidiData());
	player.ConfigureToPlayAllTracks();

	AudioDataLib::Error error;
	if (player.Setup(error))
	{
		while (!player.NoMoreToPlay() && !this->exitSignaled)
		{
			if (!player.Process(error))
				break;
		}

		player.Shutdown(error);
	}
}