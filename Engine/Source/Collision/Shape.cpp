#include "Shape.h"
#include "Shapes/Box.h"
#include "Shapes/Capsule.h"
#include "Shapes/Polygon.h"
#include "Shapes/Sphere.h"

using namespace Imzadi;
using namespace Imzadi::Collision;

std::atomic<ShapeID> Shape::nextShapeID(1);

//---------------------------------- Shape ----------------------------------

Shape::Shape()
{
	this->node = nullptr;
	this->debugColor.SetComponents(1.0, 0.0, 0.0);
	this->shapeID = nextShapeID++;
	this->cache = nullptr;
	this->objectToWorld.SetIdentity();
	this->revisionNumber = 0;
	this->userFlags = 0;
}

/*virtual*/ Shape::~Shape()
{
	delete this->cache;
}

ShapeCache* Shape::GetCache() const
{
	if (!this->cache)
		this->cache = this->CreateCache();

	if (!this->cache->isValid)
		this->cache->Update(this);

	return this->cache;
}

ShapeID Shape::GetShapeID() const
{
	return this->shapeID;
}

/*static*/ Shape* Shape::Create(TypeID typeID)
{
	switch (typeID)
	{
	case TypeID::BOX:
		return new BoxShape();
	case TypeID::CAPSULE:
		return new CapsuleShape();
	case TypeID::POLYGON:
		return new PolygonShape();
	case TypeID::SPHERE:
		return new SphereShape();
	}

	return nullptr;
}

/*static*/ std::string Shape::ShapeTypeLabel(TypeID typeID)
{
	switch (typeID)
	{
	case TypeID::BOX:
		return "BOX";
	case TypeID::CAPSULE:
		return "CAPSULE";
	case TypeID::POLYGON:
		return "POLYGON";
	case TypeID::SPHERE:
		return "SPHERE";
	}

	return "?";
}

/*virtual*/ bool Shape::Copy(const Shape* shape)
{
	// We only copy here the defining characteristics of the shape.
	// Everything else should be unique to the shape instance.
	this->objectToWorld = shape->objectToWorld;
	this->debugColor = shape->debugColor;
	this->userFlags = shape->userFlags;

	return true;
}

/*virtual*/ bool Shape::IsValid() const
{
	if (!this->objectToWorld.IsValid())
		return false;

	if (!this->GetCache()->worldToObject.IsValid())
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
	this->GetCache()->isValid = false;
	this->BumpRevisionNumber();
}

const Transform& Shape::GetObjectToWorldTransform() const
{
	return this->objectToWorld;
}

const Transform& Shape::GetWorldToObjectTransform() const
{
	return this->GetCache()->worldToObject;
}

const AxisAlignedBoundingBox& Shape::GetBoundingBox() const
{
	return this->GetCache()->boundingBox;
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

//---------------------------------- ShapeCache ----------------------------------

ShapeCache::ShapeCache()
{
	this->isValid = false;
}

/*virtual*/ ShapeCache::~ShapeCache()
{
}

/*virtual*/ void ShapeCache::Update(const Shape* shape)
{
	this->worldToObject.Invert(shape->objectToWorld);
	this->isValid = true;
}