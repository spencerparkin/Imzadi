#pragma once

#include "Entity.h"

namespace Imzadi
{
	class Biped;

	/**
	 * An instance of this class represents the level being played in the game.
	 */
	class IMZADI_API Level : public Entity
	{
	public:
		Level();
		virtual ~Level();

		/**
		 * Spawn entities and populate the scene and collision world.
		 */
		virtual bool Setup() override;

		/**
		 * Wipe the scene and collision world.
		 */
		virtual bool Shutdown() override;

		/**
		 * This is where we might, for example, animate a platform moving
		 * up and down or side-to-side to make jumping from platform to
		 * platform more challenging.  Really, the sky is the limit here
		 * for what might tick in the level.
		 */
		virtual bool Tick(TickPass tickPass, double deltaTime) override;

		/**
		 * Override this to spawn the character being played in the level.
		 */
		virtual Biped* SpawnMainCharacter();

		void SetLevelNumber(int levelNumber) { this->levelNumber = levelNumber; }
		int GetLevelNumber() { return this->levelNumber; }

	private:
		int levelNumber;
	};
}