#include "Entity.h"

using namespace Collision;

Entity::Entity()
{
	this->state = State::NEWLY_CREATED;
}

/*virtual*/ Entity::~Entity()
{
}

/*virtual*/ bool Entity::Setup()
{
	return true;
}

/*virtual*/ bool Entity::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ void Entity::Tick(double deltaTime)
{
}

/*virtual*/ bool Entity::GetTransform(Transform& transform)
{
	return false;
}