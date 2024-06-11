#pragma once

#include "Entity.h"

/**
 * This is any entity that has mass and maintains a state of current velocity.
 * There is nothing fancy going on here at all.  No intertia tensor, for example.
 * These types of entities can be projectiles or just occationally act as such.
 */
class PhysicsEntity : public Entity
{
public:
	PhysicsEntity();
	virtual ~PhysicsEntity();

	virtual bool Setup() override;
	virtual bool Tick(TickPass tickPass, double deltaTime) override;

	virtual void AccumulateForces(Collision::Vector3& netForce);
	virtual void IntegrateVelocity(const Collision::Vector3& acceleration, double deltaTime);
	virtual void Reset();

protected:
	Collision::Vector3 velocity;
	double mass;
};