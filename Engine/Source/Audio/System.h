#pragma once

#include "Defines.h"
#include "Assets/Audio.h"
#include "Math/Interval.h"
#include <xaudio2.h>
#include <string>
#include <map>
#include <set>

template<>
struct std::hash<WAVEFORMATEX>
{
	std::size_t operator()(const WAVEFORMATEX& waveFormat) const
	{
		return Imzadi::HashBuffer((const char*)&waveFormat, sizeof(waveFormat));
	}
};

inline bool operator==(const WAVEFORMATEX& waveFormatA, const WAVEFORMATEX& waveFormatB)
{
	return 0 == ::memcmp(&waveFormatA, &waveFormatB, sizeof(WAVEFORMATEX));
}

namespace Imzadi
{
	/**
	 * An instance of this class represents the audio sub-system.  Any playback
	 * of any audio of any kind goes through this system.
	 * 
	 * TODO: Can we support 3D sounds?  (Sounds that seem to be coming from a certain direction and get louder as you approach.)
	 */
	class IMZADI_API AudioSystem
	{
	public:
		AudioSystem();
		virtual ~AudioSystem();

		bool Initialize();
		bool Shutdown();
		void Tick(double deltaTimeSeconds);

		/**
		 * Load all audio assets found in the given directory.
		 * A sound can't be played unless it's been loaded before-hand.
		 * Loading a sound, of course, doesn't play it, but makes it
		 * ready to be played at a moment's notice.
		 * 
		 * @param[in] audioDirectory This is a path to the directory containing audio assets.  If not fully-qualified, we try to resolve it.
		 * @param[in] recursive If given as true, we will descend into sub-folders recursively.
		 * @return True is returned on success; false, otherwise.
		 */
		bool LoadAudioDirectory(const std::string& audioDirectory, bool recursive);

		/**
		 * Add a sound that will be occationally played in the given frequency range.
		 * 
		 * @param[in] sound This is the set of sounds to occationally play.  After each delay, a single sound is selected from this set at random.
		 * @param[in] delayRange The delay, in seconds, between occational plays is randomly selected from within this range.
		 * @param[in] startNow If true, a sound plays immediately.  If false, a initial delay is enforced before a sound plays.
		 * @return True is returned on success; false, otherwise.
		 */
		bool AddAmbientSound(const std::set<std::string>& soundSet, const Interval& delayRange, bool startNow, float volume);

		/**
		 * Stop playing ambient sounds.  They don't die immediately upon invocation of this call.
		 */
		void ClearAllAmbientSounds();

		/**
		 * Simply play the given sound until it terminates.
		 * 
		 * @param True is returned on success; false, otherwise.  Failure can occur if a sound by the given name isn't loaded.
		 */
		bool PlaySound(const std::string& sound, float volume = 1.0f);

	private:
		IXAudio2* audio;
		IXAudio2MasteringVoice* masteringVoice;
		typedef std::map<std::string, Reference<AudioSystemAsset>> AudioMap;
		AudioMap audioMap;

		struct AmbientSound
		{
			std::set<std::string> soundSet;
			Interval delayRangeSeconds;
			double waitTimeSeconds;
			double elapsedTimeSeconds;
			float volume;
		};

		std::vector<AmbientSound> ambientSoundArray;

		class AudioSource : public IXAudio2VoiceCallback
		{
		public:
			AudioSource();
			virtual ~AudioSource();

			virtual void OnVoiceProcessingPassStart(UINT32 bytesRequired) override;
			virtual void OnVoiceProcessingPassEnd() override;
			virtual void OnStreamEnd() override;
			virtual void OnBufferStart(void* bufferContext) override;
			virtual void OnBufferEnd(void* bufferContext) override;
			virtual void OnLoopEnd(void* bufferContext) override;
			virtual void OnVoiceError(void* bufferContext, HRESULT Error) override;

		public:
			WAVEFORMATEX waveFormat;
			IXAudio2SourceVoice* sourceVoice;
			bool inUse;
		};

		class AudioSourceList : public ReferenceCounted
		{
		public:
			AudioSourceList();
			virtual ~AudioSourceList();

			void Clear();
			AudioSource* GetOrCreateUnusedAudioSource(const WAVEFORMATEX& waveFormat);

		public:
			std::list<AudioSource*> audioSourceList;
		};

		typedef std::unordered_map<WAVEFORMATEX, Reference<AudioSourceList>> AudioSourceCache;
		AudioSourceCache audioSourceCache;

		void ClearSourceCache();
		AudioSourceList* GetOrCreateAudioSourceList(const WAVEFORMATEX& waveFormat);
	};
}