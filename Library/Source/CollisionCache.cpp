#include "CollisionCache.h"
#include "CollisionCalculator.h"
#include "Shape.h"
#include "Error.h"
#include <format>

using namespace Collision;

//----------------------------- CollisionCache -----------------------------

CollisionCache::CollisionCache()
{
	this->cacheMap = new ShapePairCollisionStatusMap();
	this->calculatorMap = new CollisionCalculatorMap();

	this->AddCalculator<SphereSphereCollisionCalculator>(Shape::TypeID::SPHERE, Shape::TypeID::SPHERE);
	// TODO: Add more calculators here.
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
			CollisionCalculator* calculator = calculatorIter->second;
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
	return (uint64_t(COLL_SYS_MIN(typeIDA, typeIDB)) << 32) | uint64_t(COLL_SYS_MAX(typeIDA, typeIDB));
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
		CollisionCalculator* calculator = iter->second;
		delete calculator;
		this->calculatorMap->erase(iter);
	}
}

//----------------------------- ShapePairCollisionStatus -----------------------------

ShapePairCollisionStatus::ShapePairCollisionStatus()
{
	this->inCollision = false;
	this->shapeA = nullptr;
	this->shapeB = nullptr;
	this->revisionNumberA = 0;
	this->revisionNumberB = 0;
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