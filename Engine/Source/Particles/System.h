#pragma once

#include "Defines.h"

namespace Imzadi
{
	/**
	 * TODO: Write this.
	 * 
	 * Systems like this aren't too hard to write, but what might be
	 * more difficult is providing a way to author the FX.  A live-
	 * authoring approach that utilizes the engine is a good approach.
	 * A tool that embeds the engine is probably the best way to go.
	 * Can we use instancing in the GPU?  This is where we visit the
	 * same vertex 4 times to build a quad per vertex.
	 */
	class IMZADI_API ParticleSystem
	{
	public:
		ParticleSystem();
		virtual ~ParticleSystem();
	};
}