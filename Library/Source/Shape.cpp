#include "Shape.h"

using namespace Collision;

ShapeID Shape::nextShapeID = 0;

Shape::Shape()
{
	this->debugColor.SetComponents(1.0, 1.0, 1.0);
	this->shapeID = nextShapeID++;
	this->worldToObjectValid = false;
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

/*virtual*/ bool Shape::IsValid() const
{
	if (!this->objectToWorld.IsValid())
		return false;

	this->GetWorldToObjectTransform();

	if (!this->worldToObject.IsValid())
		return false;

	return true;
}

void Shape::SetObjectToWorldTransform(const Transform& objectToWorld)
{
	this->objectToWorld = objectToWorld;
	this->worldToObjectValid = false;
}

const Transform& Shape::GetObjectToWorldTransform() const
{
	return this->objectToWorld;
}

const Transform& Shape::GetWorldToObjectTransform() const
{
	if (!this->worldToObjectValid)
	{
		this->worldToObject = this->objectToWorld.Inverted();
		this->worldToObjectValid = true;
	}

	return this->worldToObject;
}