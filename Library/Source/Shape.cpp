#include "Shape.h"
#include "Shapes/Box.h"
#include "Shapes/Capsule.h"
#include "Shapes/Polygon.h"
#include "Shapes/Sphere.h"

using namespace Collision;

std::atomic<ShapeID> Shape::nextShapeID(1);

Shape::Shape(bool temporary)
{
	this->node = nullptr;
	this->debugColor.SetComponents(1.0, 0.0, 0.0);
	this->shapeID = temporary ? 0 : nextShapeID++;
	this->cacheValid = false;
	this->objectToWorld.SetIdentity();
	this->revisionNumber = 0;
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

/*static*/ Shape* Shape::Create(TypeID typeID)
{
	switch (typeID)
	{
	case TypeID::BOX:
		return new BoxShape(false);
	case TypeID::CAPSULE:
		return new CapsuleShape(false);
	case TypeID::POLYGON:
		return new PolygonShape(false);
	case TypeID::SPHERE:
		return new SphereShape(false);
	}

	return nullptr;
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

/*virtual*/ bool Shape::Split(const Plane& plane, Shape*& shapeBack, Shape*& shapeFront) const
{
	return false;
}

void Shape::SetObjectToWorldTransform(const Transform& objectToWorld)
{
	this->objectToWorld = objectToWorld;
	this->cacheValid = false;
	this->BumpRevisionNumber();
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

/*virtual*/ bool Shape::Dump(std::ostream& stream) const
{
	this->objectToWorld.Dump(stream);
	this->debugColor.Dump(stream);
	return true;
}

/*virtual*/ bool Shape::Restore(std::istream& stream)
{
	this->objectToWorld.Restore(stream);
	this->debugColor.Restore(stream);
	return true;
}