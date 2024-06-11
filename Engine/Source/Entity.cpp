#include "Entity.h"

using namespace Imzadi;

Entity::Entity()
{
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

/*virtual*/ bool Entity::Tick(TickPass tickPass, double deltaTime)
{
	return true;
}

/*virtual*/ bool Entity::GetTransform(Transform& transform)
{
	return false;
}