#include "CollisionPairCalculator.h"

using namespace Collision;

//-------------------------------------- CollisionPairCalculator --------------------------------------

CollisionPairCalculator::CollisionPairCalculator()
{
}

/*virtual*/ CollisionPairCalculator::~CollisionPairCalculator()
{
}

//-------------------------------------- SphereSphereCollisionPairCalculator --------------------------------------

SphereSphereCollisionPairCalculator::SphereSphereCollisionPairCalculator()
{
}

/*virtual*/ SphereSphereCollisionPairCalculator::~SphereSphereCollisionPairCalculator()
{
}

/*virtual*/ CollisionPair* SphereSphereCollisionPairCalculator::Calculate(Shape* shapeA, Shape* shapeB)
{
	return nullptr;
}