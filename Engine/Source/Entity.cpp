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

/*virtual*/ uint32_t Entity::TickOrder() const
{
	return 0;
}

/*virtual*/ bool Entity::OwnsCollisionShape(ShapeID shapeID) const
{
	return false;
}