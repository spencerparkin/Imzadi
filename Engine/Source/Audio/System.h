#pragma once

#include "Defines.h"
#include <xaudio2.h>

namespace Imzadi
{
	/**
	 * This is our abstraction layer against the audio sub-system.
	 */
	class IMZADI_API AudioSystem
	{
	public:
		AudioSystem();
		virtual ~AudioSystem();

		bool Initialize();
		bool Shutdown();

	private:
		IXAudio2* audio;
		IXAudio2MasteringVoice* masteringVoice;
	};
}