#include "PhysicsEntity.h"
#include "Game.h"

using namespace Collision;

PhysicsEntity::PhysicsEntity()
{
	this->velocity.SetComponents(0.0, 0.0, 0.0);
	this->mass = 1.0;
}

/*virtual*/ PhysicsEntity::~PhysicsEntity()
{
}

/*virtual*/ bool PhysicsEntity::Setup()
{
	this->velocity.SetComponents(0.0, 0.0, 0.0);
	this->mass = 1.0;
	return true;
}

/*virtual*/ void PhysicsEntity::AccumulateForces(Collision::Vector3& netForce)
{
	Vector3 downVector(0.0, -1.0, 0.0);
	Vector3 gravityForce = downVector * this->mass * Game::Get()->GetGravity();
	//netForce += gravityForce;		TODO: Uncomment when ready.
}

/*virtual*/ void PhysicsEntity::IntegrateVelocity(const Collision::Vector3& acceleration, double deltaTime)
{
	this->velocity += acceleration * deltaTime;
}

/*virtual*/ bool PhysicsEntity::Tick(TickPass tickPass, double deltaTime)
{
	if (tickPass == TickPass::PRE_TICK)
	{
		Vector3 netForce(0.0, 0.0, 0.0);
		this->AccumulateForces(netForce);

		Vector3 acceleration = netForce / this->mass;
		this->IntegrateVelocity(acceleration, deltaTime);
	}

	return true;
}

/*virtual*/ void PhysicsEntity::Reset()
{
	this->velocity.SetComponents(0.0, 0.0, 0.0);
}