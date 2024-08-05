#pragma once

#include "Defines.h"
#include "Assets/Audio.h"
#include <xaudio2.h>
#include <string>
#include <map>
#include <set>

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
		 * This will kick-off the playing of all sounds in the given set,
		 * not at the same time, but one after another, randomly, with the
		 * sounds fading in and out, never letting there be a moment of
		 * of silence between sounds.
		 * 
		 * @param[in] ambientSoundsSet This should be a set of pre-loaded sounds from which the system chooses at random.  If the set is empty, all ambient sounds stop.
		 * @param True is returned on success; false, otherwise.  Failure can occur if a sound by the given name isn't loaded.
		 */
		bool PlayAmbientSounds(const std::set<std::string>& ambientSoundsSet);

		/**
		 * This will kick-off the playing of the given sound at random times
		 * governed by the given frequency range.  You might, for example,
		 * use this to play an owl sound.
		 * 
		 * @param[in] ambientSound This is the sound to play.  If the frequency range given is [0,0], then the sound stops playing.
		 * @param[in] minFrequency In units of plays per minute, this is the soonest the sound will play after it's previous ending.
		 * @param[in] maxFrequency In units of plays per minute, this is the latest the sound will play after it's previous ending.
		 * @param True is returned on success; false, otherwise.  Failure can occur if a sound by the given name isn't loaded.
		 */
		bool PlayAmbientSoundOccationally(const std::string& ambientSound, double minFrequency, double maxFrequency);

		/**
		 * Simply play the given sound until it terminates.
		 * 
		 * @param True is returned on success; false, otherwise.  Failure can occur if a sound by the given name isn't loaded.
		 */
		bool PlaySound(const std::string& sound);

	private:
		IXAudio2* audio;
		IXAudio2MasteringVoice* masteringVoice;
		typedef std::map<std::string, Reference<Audio>> AudioMap;
		AudioMap audioMap;
	};
}