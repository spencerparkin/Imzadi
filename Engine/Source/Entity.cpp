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

/*virtual*/ bool Entity::Shutdown()
{
	return true;
}

/*virtual*/ bool Entity::Tick(TickPass tickPass, double deltaTime)
{
	return true;
}

/*virtual*/ bool Entity::GetTransform(Transform& transform) const
{
	return false;
}

/*virtual*/ bool Entity::SetTransform(const Imzadi::Transform& transform)
{
	return false;
}

/*virtual*/ uint32_t Entity::ShutdownOrder() const
{
	return 0;
}