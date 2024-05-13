#include "CollisionCache.h"
#include "CollisionCalculator.h"
#include "Shape.h"
#include "Shapes/Sphere.h"
#include "Shapes/Box.h"
#include "Shapes/Capsule.h"
#include "Shapes/Polygon.h"
#include "Error.h"
#include <format>

using namespace Collision;

//----------------------------- CollisionCache -----------------------------

CollisionCache::CollisionCache()
{
	this->cacheMap = new ShapePairCollisionStatusMap();
	this->calculatorMap = new CollisionCalculatorMap();

	// Sphere:
	this->AddCalculator<SphereShape, SphereShape>();
	this->AddCalculator<SphereShape, CapsuleShape>();
	this->AddCalculator<SphereShape, PolygonShape>();
	this->AddCalculator<SphereShape, BoxShape>();

	// Capsule:
	this->AddCalculator<CapsuleShape, SphereShape>();
	this->AddCalculator<CapsuleShape, CapsuleShape>();
	this->AddCalculator<CapsuleShape, PolygonShape>();
	this->AddCalculator<CapsuleShape, BoxShape>();
	
	// Polygon:
	this->AddCalculator<PolygonShape, SphereShape>();
	this->AddCalculator<PolygonShape, CapsuleShape>();
	this->AddCalculator<PolygonShape, PolygonShape>();
	this->AddCalculator<PolygonShape, BoxShape>();

	// Box:
	this->AddCalculator<BoxShape, SphereShape>();
	this->AddCalculator<BoxShape, CapsuleShape>();
	this->AddCalculator<BoxShape, PolygonShape>();
	this->AddCalculator<BoxShape, BoxShape>();
}

/*virtual*/ CollisionCache::~CollisionCache()
{
	this->Clear();
	this->ClearCalculatorMap();

	delete this->cacheMap;
	delete this->calculatorMap;
}

ShapePairCollisionStatus* CollisionCache::DetermineCollisionStatusOfShapes(const Shape* shapeA, const Shape* shapeB)
{
	ShapePairCollisionStatus* collisionStatus = nullptr;

	std::string cacheKey = this->MakeCacheKey(shapeA, shapeB);

	ShapePairCollisionStatusMap::iterator cacheIter = this->cacheMap->find(cacheKey);
	if (cacheIter != this->cacheMap->end())
	{
		collisionStatus = cacheIter->second;
		if (!collisionStatus->IsValid())
		{
			delete collisionStatus;
			collisionStatus = nullptr;
			this->cacheMap->erase(cacheIter);
		}
	}
	
	if (!collisionStatus)
	{
		uint64_t calculatorKey = this->MakeCalculatorKey(shapeA, shapeB);
		CollisionCalculatorMap::iterator calculatorIter = this->calculatorMap->find(calculatorKey);
		if (calculatorIter == this->calculatorMap->end())
		{
			GetError()->AddErrorMessage(std::format("No collision calculator exists for shapes of type ID {} and {}.", uint32_t(shapeA->GetShapeTypeID()), uint32_t(shapeB->GetShapeTypeID())));
		}
		else
		{
			CollisionCalculatorInterface* calculator = calculatorIter->second;
			collisionStatus = calculator->Calculate(shapeA, shapeB);
			if (!collisionStatus)
			{
				GetError()->AddErrorMessage(std::format("Collision calculator failed to calculate collision status for shapes {} and {}", shapeA->GetShapeID(), shapeB->GetShapeID()));
			}
			else
			{
				this->cacheMap->insert(std::pair<std::string, ShapePairCollisionStatus*>(cacheKey, collisionStatus));
			}
		}
	}

	return collisionStatus;
}

std::string CollisionCache::MakeCacheKey(const Shape* shapeA, const Shape* shapeB)
{
	ShapeID shapeIDs[2];

	shapeIDs[0] = COLL_SYS_MIN(shapeA->GetShapeID(), shapeB->GetShapeID());
	shapeIDs[1] = COLL_SYS_MAX(shapeA->GetShapeID(), shapeB->GetShapeID());

	return std::format("{}-{}", shapeIDs[0], shapeIDs[1]);
}

uint64_t CollisionCache::MakeCalculatorKey(const Shape* shapeA, const Shape* shapeB)
{
	uint32_t typeIDA = shapeA->GetShapeTypeID();
	uint32_t typeIDB = shapeB->GetShapeTypeID();

	return this->MakeCalculatorKey(typeIDA, typeIDB);
}

uint64_t CollisionCache::MakeCalculatorKey(uint32_t typeIDA, uint32_t typeIDB)
{
	return (uint64_t(typeIDA) << 32) | uint64_t(typeIDB);
}

void CollisionCache::Clear()
{
	while (this->cacheMap->size() > 0)
	{
		ShapePairCollisionStatusMap::iterator iter = this->cacheMap->begin();
		ShapePairCollisionStatus* collisionStatus = iter->second;
		delete collisionStatus;
		this->cacheMap->erase(iter);
	}
}

void CollisionCache::ClearCalculatorMap()
{
	while (this->calculatorMap->size() > 0)
	{
		CollisionCalculatorMap::iterator iter = this->calculatorMap->begin();
		CollisionCalculatorInterface* calculator = iter->second;
		delete calculator;
		this->calculatorMap->erase(iter);
	}
}

//----------------------------- ShapePairCollisionStatus -----------------------------

ShapePairCollisionStatus::ShapePairCollisionStatus(const Shape* shapeA, const Shape* shapeB)
{
	this->inCollision = false;
	this->shapeA = shapeA;
	this->shapeB = shapeB;
	this->revisionNumberA = shapeA->GetRevisionNumber();
	this->revisionNumberB = shapeB->GetRevisionNumber();
}

/*virtual*/ ShapePairCollisionStatus::~ShapePairCollisionStatus()
{
}

bool ShapePairCollisionStatus::IsValid() const
{
	if (this->shapeA->GetRevisionNumber() != this->revisionNumberA)
		return false;

	if (this->shapeB->GetRevisionNumber() != this->revisionNumberB)
		return false;

	return true;
}

ShapeID ShapePairCollisionStatus::GetShapeID(int i) const
{
	if (i % 2 == 0)
		return this->shapeA->GetShapeID();
	else
		return this->shapeB->GetShapeID();
}

Vector3 ShapePairCollisionStatus::GetSeparationDelta(ShapeID shapeID) const
{
	if (shapeID == this->shapeA->GetShapeID())
		return this->separationDelta;
	else if (shapeID == this->shapeB->GetShapeID())
		return -this->separationDelta;

	return Vector3(0.0, 0.0, 0.0);
}

double ShapePairCollisionStatus::GetSeparationDeltaLength() const
{
	if (!this->inCollision)
		return 0.0;

	return this->separationDelta.Length();
}