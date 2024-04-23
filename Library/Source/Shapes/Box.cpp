#include "Box.h"

using namespace Collision;

BoxShape::BoxShape()
{
}

/*virtual*/ BoxShape::~BoxShape()
{
}

/*virtual*/ Shape::TypeID BoxShape::GetShapeTypeID() const
{
	return TypeID::BOX;
}

/*virtual*/ void BoxShape::CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	// TODO: Write this.
}

/*virtual*/ bool BoxShape::IsValid() const
{
	// TODO: Write this.
	return false;
}

/*virtual*/ double BoxShape::CalcSize() const
{
	// TODO: Write this.
	return 0.0;
}