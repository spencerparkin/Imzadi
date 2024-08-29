#pragma once

#include "Entity.h"
#include "Collision/Shape.h"
#include "RenderObjects/RenderMeshInstance.h"

/**
 * This is anything you can collect in the game by overlapping it and
 * then pressing a button to add it to your inventory.
 */
class Pickup : public Imzadi::Entity
{
public:
	Pickup();
	virtual ~Pickup();

	/**
	 * Get the pick-up rendering in the world with a collision shape.
	 */
	virtual bool Setup();

	/**
	 * Remove this pick-up from the scene and from the collision world.
	 */
	virtual bool Shutdown();

	/**
	 * This will just slowsly rotate the rendered pick-up.  The main character
	 * will be responsible for the collision detection, and registering the collection
	 * action (bound to a button).
	 */
	virtual bool Tick(Imzadi::TickPass tickPass, double deltaTime) override;

	/**
	 * Set the transform that will be used when the pick-up is created.
	 * Once created, the pick-up location can't be changed, so this call doesn't do anything.
	 */
	virtual bool SetTransform(const Imzadi::Transform& transform) override;

	/**
	 * Derivatives must override this to perform the collection process.
	 * That is, to add something to the user's inventory, or do whatever
	 * else makes sense when the pickup is collected.  By default, here
	 * we just destroy the entity so that it can be removed from the scene.
	 * Overrides should call this base class method.
	 */
	virtual void Collect();

protected:
	Imzadi::Transform initialTransform;		///< Where the pickup is initially placed.
	std::string renderMeshFile;				///< Derivative should fill this out with the applicable render mesh asset to use.
	Imzadi::Collision::ShapeID shapeID;		///< This is how we track our collision shape in the collision system.
	Imzadi::Reference<Imzadi::RenderMeshInstance> renderMesh;	///< This is what we're rendering in the scene.
};

/**
 * When collected, this just increments the number of lives the player has.
 */
class ExtraLifePickup : public Pickup
{
public:
	ExtraLifePickup();
	virtual ~ExtraLifePickup();

	virtual void Collect() override;
};

/**
 * Once added to the player's inventory, they can activate a speed-boost
 * (with a button press) at any desired time.  Doing so, they'll move extra
 * speed-fast for some amount of time.
 *
 * Some obstancles, for example, can't be overcome at normal speed.  So finding
 * and using one of these pick-ups is a way the player solves a level.
 */
class SpeedBoostPickup : public Pickup
{
public:
	SpeedBoostPickup();
	virtual ~SpeedBoostPickup();

	virtual void Collect() override;
};