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
	 * This can be optionally overridden to provide more information about the pickup.
	 */
	virtual void Configure(const std::unordered_map<std::string, std::string>& configMap);

	/**
	 * Tell the caller if we own the given collision shape.  This will be
	 * needed during the collision detection process for getting the pickup.
	 */
	virtual bool OwnsCollisionShape(Imzadi::Collision::ShapeID shapeID) const override;

	/**
	 * Derivatives must override this to perform the collection process.
	 * That is, to add something to the user's inventory, or do whatever
	 * else makes sense when the pickup is collected.  By default, here
	 * we just destroy the entity so that it can be removed from the scene.
	 * Overrides should call this base class method unless they're not
	 * supposed to disappear when "collected".
	 */
	virtual void Collect();

	/**
	 * Get a label meant to be presented to the player that identifies
	 * what kind of pickup this is.
	 */
	virtual std::string GetLabel() const = 0;

	/**
	 * By default, this returns "pickup".  But some pickups don't
	 * disappear when when you "collect" them.
	 */
	virtual std::string GetVerb() const;

	/**
	 * There can be reasons why a pick-up can't be collected.
	 * Think "the sword and the stone", for instance.  By default,
	 * we return true here.
	 */
	virtual bool CanBePickedUp() const;

protected:
	Imzadi::Transform initialTransform;			///< Where the pickup is initially placed.
	std::string renderMeshFile;					///< Derivative should fill this out with the applicable render mesh asset to use.
	Imzadi::Collision::ShapeID pickupShapeID;	///< This is how we track our collision shape in the collision system.
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
	virtual std::string GetLabel() const override;
};

/**
 * Once added to the player's inventory, they can activate a speed-boost
 * (with a button press) at any desired time.  Doing so, they'll move
 * faster than usual for some amount of time.
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
	virtual std::string GetLabel() const override;
};

/**
 * This pickup simply kicks-off a MIDI song.  That's mostly for fun,
 * but I think I can also make it part of solving a level.
 */
class SongPickup : public Pickup
{
public:
	SongPickup();
	virtual ~SongPickup();

	virtual void Collect() override;
	virtual std::string GetLabel() const override;
	virtual std::string GetVerb() const override;
	virtual void Configure(const std::unordered_map<std::string, std::string>& configMap) override;
	virtual bool CanBePickedUp() const override;

private:
	std::string song;
};

/**
 * This pickup simply adds a key to your inventory.
 */
class KeyPickup : public Pickup
{
public:
	KeyPickup();
	virtual ~KeyPickup();

	virtual void Collect() override;
	virtual std::string GetLabel() const override;
};

/**
 * Collect all of these and return them to me and you win the game.  Yay.
 */
class BenzoPickup : public Pickup
{
public:
	BenzoPickup();
	virtual ~BenzoPickup();

	virtual bool Setup() override;
	virtual void Collect() override;
	virtual std::string GetLabel() const override;
	virtual void Configure(const std::unordered_map<std::string, std::string>& configMap) override;

	static bool IsBenzoName(const std::string& name);

private:
	std::string benzoType;
};