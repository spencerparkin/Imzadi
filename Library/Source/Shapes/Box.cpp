#include "Box.h"
#include "Math/AxisAlignedBoundingBox.h"
#include <vector>

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
	std::vector<Vector3> boxPoints;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				Vector3 cornerVector;

				cornerVector.x = this->extents.x * (i == 0) ? -1.0 : 1.0;
				cornerVector.y = this->extents.y * (j == 0) ? -1.0 : 1.0;
				cornerVector.z = this->extents.z * (k == 0) ? -1.0 : 1.0;

				boxPoints.push_back(this->objectToWorld.TransformPoint(cornerVector));
			}
		}
	}

	boundingBox.SetToBoundPointCloud(boxPoints);
}

/*virtual*/ bool BoxShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (!this->extents.IsValid())
		return false;

	return true;
}

/*virtual*/ double BoxShape::CalcSize() const
{
	return 8.0 * this->extents.x * this->extents.y * this->extents.z;
}

/*virtual*/ void BoxShape::DebugRender(DebugRenderResult* renderResult) const
{
}