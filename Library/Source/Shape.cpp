#include "Shape.h"

using namespace Collision;

ShapeID Shape::nextShapeID = 0;

Shape::Shape()
{
	this->shapeID = nextShapeID++;
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