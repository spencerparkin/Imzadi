#pragma once

#include "Defines.h"

namespace Imzadi
{
	/**
	 * TODO: Write this.
	 * 
	 * Here I'd like an easy way to fire-and-forget sound-FX.  Can we place them
	 * in the world so that they sound like they're coming from a certain direction?
	 * We should also be able to stream music.  I've done my own audio programming
	 * and mixing before, but haven't tried to use XAudio2.  Can I leverage this to
	 * do most of the heavy lifting of dealing with the audio streams?
	 */
	class IMZADI_API AudioSystem
	{
	public:
		AudioSystem();
		virtual ~AudioSystem();
	};
}