#pragma once

#include "Entity.h"

namespace Imzadi
{
	/**
	 * An instance of this class represents the level being played in the game.
	 */
	class IMZADI_API Level : public Entity
	{
	public:
		Level();
		virtual ~Level();

		/**
		 * Load the here and all enemies into the level.
		 */
		virtual bool Setup() override;

		/**
		 * Clean-up the level and then spawn the next one, if any.
		 * If there is no next-level, then you win!!
		 */
		virtual bool Shutdown(bool gameShuttingDown) override;

		/**
		 * This is where we might, for example, animate a platform moving
		 * up and down or side-to-side to make jumping from platform to
		 * platform more challenging.  Really, the sky is the limit here
		 * for what might tick in the level.
		 */
		virtual bool Tick(TickPass tickPass, double deltaTime) override;

		void SetLevelNumber(int levelNumber) { this->levelNumber = levelNumber; }
		int GetLevelNumber() { return this->levelNumber; }

	private:
		int levelNumber;
	};
}