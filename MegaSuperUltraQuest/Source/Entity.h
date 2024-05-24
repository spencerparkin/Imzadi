#pragma once

#include "Reference.h"
#include "Math/Transform.h"

/**
 * These are the living things that exist in a level, moving, walking and
 * jumping around.  Most are computer-controller.  Some are human-controlled.
 * Note that an entity doesn't need to be a character or even be something
 * that is rendred in the scene.  It's just anything that needs to tick as
 * the game plays.
 */
class Entity : public ReferenceCounted
{
	friend class Game;

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
	 * @param[in] deltaTime This is the time, in seconds, between now and the last frame update.
	 */
	virtual void Tick(double deltaTime);

	/**
	 * Return the location and orientation of this entity, if applicable.
	 * This concept does not apply to all entity types.  The default implimentation
	 * just returns false.
	 */
	virtual bool GetTransform(Collision::Transform& transform);

private:
	enum State
	{
		NEWLY_CREATED,
		NEEDS_SETUP,
		NEEDS_TICK,
		NEEDS_SHUTDOWN,
		AWAITING_DELETION
	};

	State state;
};