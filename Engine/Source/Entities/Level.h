#pragma once

#include "Entity.h"

namespace Imzadi
{
	class Biped;
	class LevelData;

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

		/**
		 * Make sure we're the last entity to get shutdown when the level goes down.
		 */
		virtual uint32_t ShutdownOrder() const override;

		/**
		 */
		virtual bool SetupWithLevelData(LevelData* levelData);

		void SetLevelName(const std::string& levelName) { this->levelName = levelName; }
		const std::string& GetLevelName() const { return this->levelName; }

	private:
		std::string levelName;
	};
}