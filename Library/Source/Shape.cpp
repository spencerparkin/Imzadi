#include "Shape.h"

using namespace Collision;

ShapeID Shape::nextShapeID = 1;

Shape::Shape()
{
	this->node = nullptr;
	this->debugColor.SetComponents(1.0, 0.0, 0.0);
	this->shapeID = nextShapeID++;
	this->cacheValid = false;
	this->objectToWorld.SetIdentity();
}

/*virtual*/ Shape::~Shape()
{
}

ShapeID Shape::GetShapeID() const
{
	return this->shapeID;
}

/*static*/ void Shape::Free(Shape* shape)
{
	delete shape;
}

/*virtual*/ void Shape::RecalculateCache() const
{
	this->cache.worldToObject.Invert(this->objectToWorld);
}

void Shape::RegenerateCacheIfNeeded() const
{
	if (!this->cacheValid)
	{
		this->RecalculateCache();
		this->cacheValid = true;
	}
}

/*virtual*/ bool Shape::IsValid() const
{
	if (!this->objectToWorld.IsValid())
		return false;

	this->RegenerateCacheIfNeeded();

	if (!this->cache.worldToObject.IsValid())
		return false;

	return true;
}

void Shape::SetObjectToWorldTransform(const Transform& objectToWorld)
{
	this->objectToWorld = objectToWorld;
	this->cacheValid = false;
}

const Transform& Shape::GetObjectToWorldTransform() const
{
	return this->objectToWorld;
}

const Transform& Shape::GetWorldToObjectTransform() const
{
	this->RegenerateCacheIfNeeded();

	return this->cache.worldToObject;
}

const AxisAlignedBoundingBox& Shape::GetBoundingBox() const
{
	this->RegenerateCacheIfNeeded();

	return this->cache.boundingBox;
}