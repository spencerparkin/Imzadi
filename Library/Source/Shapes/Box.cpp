#include "Box.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Result.h"
#include <vector>

using namespace Collision;

BoxShape::BoxShape()
{
}

/*virtual*/ BoxShape::~BoxShape()
{
}

/*static*/ BoxShape* BoxShape::Create()
{
	return new BoxShape();
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
	Vector3 boxVertices[2][2][2];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				Vector3 cornerVector;

				cornerVector.x = this->extents.x * ((i == 0) ? -1.0 : 1.0);
				cornerVector.y = this->extents.y * ((j == 0) ? -1.0 : 1.0);
				cornerVector.z = this->extents.z * ((k == 0) ? -1.0 : 1.0);

				boxVertices[i][j][k] = this->objectToWorld.TransformPoint(cornerVector);
			}
		}
	}

	DebugRenderResult::RenderLine renderLine;

	renderLine.color.SetComponents(1.0, 1.0, 1.0);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			renderLine.line.point[0] = boxVertices[i][j][0];
			renderLine.line.point[1] = boxVertices[i][j][1];
			renderResult->AddRenderLine(renderLine);

			renderLine.line.point[0] = boxVertices[i][0][j];
			renderLine.line.point[1] = boxVertices[i][1][j];
			renderResult->AddRenderLine(renderLine);

			renderLine.line.point[0] = boxVertices[0][i][j];
			renderLine.line.point[1] = boxVertices[1][i][j];
			renderResult->AddRenderLine(renderLine);
		}
	}
}