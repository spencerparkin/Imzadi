#pragma once

#include "Reference.h"
#include "Game.h"
#include "Math/Transform.h"

namespace Imzadi
{
	/**
	 * These are the elements of game-play and anything that needs to be ticked.
	 */
	class IMZADI_API Entity : public ReferenceCounted
	{
	public:
		Entity();
		virtual ~Entity();

		/**
		 * This entity was just created.  Do any necessary initialization.
		 * Here the entity might add a render object to the scene.
		 */
		virtual bool Setup();

		/**
		 * This entity is going away.  Do any necessary clean-up.
		 * Here the entity might remove a render object from the scene.
		 */
		virtual bool Shutdown();

		/**
		 * Animate and/or simulate this entity.
		 *
		 * @param[in] tickPass This specifies which tick this call is for.  See the TickPass enum for more information.
		 * @param[in] deltaTime This is the time, in seconds, between now and the last frame update.
		 * @return False should be returned when the entity no longer wants to tick and must die.
		 */
		virtual bool Tick(TickPass tickPass, double deltaTime);

		/**
		 * Return the location and orientation of this entity, if applicable.
		 * This concept does not apply to all entity types.  The default implimentation
		 * just returns false.
		 */
		virtual bool GetTransform(Imzadi::Transform& transform) const;

		/**
		 * Set the location and orientation of this entity, if applicable.
		 * This concept does not apply to allentity types.  The default implimentation
		 * just returns false.
		 */
		virtual bool SetTransform(const Imzadi::Transform& transform);

		/**
		 * Typically, a level entity is spawned which, in turns, spawns other entities
		 * into the level, and so forth.  When a level is torn down, however, this method
		 * here gives some control over the order in which entities are shutdown.
		 * All that may be necessary is for the level entity to get shutdown last since
		 * it does most of the setup of the level.
		 */
		virtual uint32_t ShutdownOrder() const;

		/**
		 * This gives us some control over tick order.  For example, we need platforms
		 * to tick before entities.
		 */
		virtual uint32_t TickOrder() const;

		/**
		 * Again, this may not be applicable to all entities, but if it does, this is
		 * where the entity can say whether or not it owns the given collision shape ID.
		 * By default, we just return false here.
		 */
		virtual bool OwnsCollisionShape(Collision::ShapeID shapeID) const;

		/**
		 * Again, only if applicable, return here the collision shape representing the
		 * ground with which this entity is in contact.
		 */
		virtual Collision::ShapeID GetGroundContactShape() const;

		/**
		 * This can be implimented to provide info for the info command.
		 */
		virtual std::string GetInfo() const;

		/**
		 * Set a name for this entity that can be used to look it up in the system.
		 */
		void SetName(const std::string& name) { this->name = name; }

		/**
		 * Get the name by which this entity can be found in the system.
		 */
		const std::string& GetName() const { return this->name; }

		/**
		 * This can be used to mark an entity for immediate removal and deletion from the game.
		 */
		void DoomEntity();

		/**
		 * Tell the caller if this entity has been marked for removal and deletion from the game.
		 */
		bool IsDoomed() const { return this->blackSpot; }

	protected:
		std::string name;
		bool blackSpot;
	};
}