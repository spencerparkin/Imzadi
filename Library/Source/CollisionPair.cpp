#include "CollisionPair.h"

using namespace Collision;

CollisionPair::CollisionPair()
{
	this->shapeA = nullptr;
	this->shapeB = nullptr;
}

/*virtual*/ CollisionPair::~CollisionPair()
{
}