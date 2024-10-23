#pragma once

#include "Entity.h"
#include "Assets/LevelData.h"

namespace Imzadi
{
	class Biped;
	class NavGraph;

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
		 * Override this to spawn NPC specified in the level.
		 */
		virtual void SpawnNPC(const LevelData::NPC* npc);

		/**
		 * Make sure we're the last entity to get shutdown when the level goes down.
		 */
		virtual uint32_t ShutdownOrder() const override;

		/**
		 * This is called during setup and can be optionally overridden to do more with the data.
		 */
		virtual bool SetupWithLevelData(LevelData* levelData);

		/**
		 * Some levels may need custom collision world extents.
		 * The given box will fit the collision objects loaded during
		 * the level load, but a marge shoudl be added to the box.
		 * Some levels need an extra large margin.
		 */
		virtual void AdjustCollisionWorldExtents(AxisAlignedBoundingBox& collisionWorldBox);

		void SetLevelName(const std::string& levelName) { this->levelName = levelName; }
		const std::string& GetLevelName() const { return this->levelName; }

		NavGraph* GetNavGraph() { return this->navGraph.Get(); }

	public:
		bool debugDrawNavGraph;

	private:
		std::string levelName;

	protected:
		Reference<NavGraph> navGraph;
	};
}