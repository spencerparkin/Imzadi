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
		 *
		 * @param[in] gameShuttingDown We're shutting the whole game down, not just this entity.
		 */
		virtual bool Shutdown(bool gameShuttingDown);

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
		virtual bool GetTransform(Imzadi::Transform& transform);
	};
}