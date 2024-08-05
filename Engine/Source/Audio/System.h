#pragma once

#include "Defines.h"
#include "Assets/Audio.h"
#include <xaudio2.h>
#include <string>
#include <map>

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

	private:
		IXAudio2* audio;
		IXAudio2MasteringVoice* masteringVoice;
		typedef std::map<std::string, Reference<Audio>> AudioMap;
		AudioMap audioMap;
	};
}