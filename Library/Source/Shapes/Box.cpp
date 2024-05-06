#include "Box.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Math/Ray.h"
#include "Polygon.h"
#include "Result.h"
#include <vector>

using namespace Collision;

BoxShape::BoxShape(bool temporary) : Shape(temporary)
{
}

/*virtual*/ BoxShape::~BoxShape()
{
}

/*static*/ BoxShape* BoxShape::Create()
{
	return new BoxShape(false);
}

/*virtual*/ Shape::TypeID BoxShape::GetShapeTypeID() const
{
	return TypeID::BOX;
}

/*static*/ Shape::TypeID BoxShape::StaticTypeID()
{
	return TypeID::BOX;
}

void BoxShape::GetCornerMatrix(BoxVertexMatrix& boxVertices, bool worldSpace) const
{
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

				boxVertices[i][j][k] = worldSpace ? this->objectToWorld.TransformPoint(cornerVector) : cornerVector;
			}
		}
	}
}

void BoxShape::GetCornerPointArray(std::vector<Vector3>& cornerPointArray, bool worldSpace) const
{
	BoxVertexMatrix boxVertices;
	this->GetCornerMatrix(boxVertices, worldSpace);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
				cornerPointArray.push_back(boxVertices[i][j][k]);
}

void BoxShape::GetEdgeSegmentArray(std::vector<LineSegment>& edgeSegmentArray, bool worldSpace) const
{
	BoxVertexMatrix boxVertices;
	this->GetCornerMatrix(boxVertices, worldSpace);

	LineSegment edge;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			edge.point[0] = boxVertices[i][j][0];
			edge.point[1] = boxVertices[i][j][1];
			edgeSegmentArray.push_back(edge);

			edge.point[0] = boxVertices[i][0][j];
			edge.point[1] = boxVertices[i][1][j];
			edgeSegmentArray.push_back(edge);

			edge.point[0] = boxVertices[0][i][j];
			edge.point[1] = boxVertices[1][i][j];
			edgeSegmentArray.push_back(edge);
		}
	}
}

void BoxShape::GetFacePolygonArray(std::vector<PolygonShape>& facePolygonArray, bool worldSpace) const
{
	BoxVertexMatrix boxVertices;
	this->GetCornerMatrix(boxVertices, worldSpace);

	PolygonShape face(true);
	face.AddVertex(boxVertices[0][0][0]);
	face.AddVertex(boxVertices[0][0][1]);
	face.AddVertex(boxVertices[0][1][1]);
	face.AddVertex(boxVertices[0][1][0]);
	facePolygonArray.push_back(face);

	face.Clear();
	face.AddVertex(boxVertices[1][0][0]);
	face.AddVertex(boxVertices[1][1][0]);
	face.AddVertex(boxVertices[1][1][1]);
	face.AddVertex(boxVertices[1][0][1]);
	facePolygonArray.push_back(face);

	face.Clear();
	face.AddVertex(boxVertices[0][0][0]);
	face.AddVertex(boxVertices[0][0][1]);
	face.AddVertex(boxVertices[1][0][1]);
	face.AddVertex(boxVertices[1][0][0]);
	facePolygonArray.push_back(face);

	face.Clear();
	face.AddVertex(boxVertices[0][1][0]);
	face.AddVertex(boxVertices[1][1][0]);
	face.AddVertex(boxVertices[1][1][1]);
	face.AddVertex(boxVertices[0][1][1]);
	facePolygonArray.push_back(face);

	face.Clear();
	face.AddVertex(boxVertices[0][0][0]);
	face.AddVertex(boxVertices[0][1][0]);
	face.AddVertex(boxVertices[1][1][0]);
	face.AddVertex(boxVertices[1][0][0]);
	facePolygonArray.push_back(face);

	face.Clear();
	face.AddVertex(boxVertices[0][0][1]);
	face.AddVertex(boxVertices[1][0][1]);
	face.AddVertex(boxVertices[1][1][1]);
	face.AddVertex(boxVertices[0][1][1]);
	facePolygonArray.push_back(face);
}

/*virtual*/ void BoxShape::RecalculateCache() const
{
	Shape::RecalculateCache();

	std::vector<Vector3> cornerPointArray;
	this->GetCornerPointArray(cornerPointArray, true);
	this->cache.boundingBox.SetToBoundPointCloud(cornerPointArray);
}

/*virtual*/ bool BoxShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (!this->extents.IsValid())
		return false;

	return true;
}

void BoxShape::GetAxisAlignedBox(AxisAlignedBoundingBox& box) const
{
	box.minCorner = -this->extents;
	box.maxCorner = this->extents;
}

/*virtual*/ double BoxShape::CalcSize() const
{
	return 8.0 * this->extents.x * this->extents.y * this->extents.z;
}

/*virtual*/ bool BoxShape::ContainsPoint(const Vector3& point) const
{
	Transform worldToObject = this->GetWorldToObjectTransform();

	Vector3 objectSpacePoint = worldToObject.TransformPoint(point);

	AxisAlignedBoundingBox box;
	this->GetAxisAlignedBox(box);

	return box.ContainsPoint(objectSpacePoint);
}

/*virtual*/ void BoxShape::DebugRender(DebugRenderResult* renderResult) const
{
	std::vector<LineSegment> edgeSegmentArray;
	this->GetEdgeSegmentArray(edgeSegmentArray, true);

	for (const LineSegment& edge : edgeSegmentArray)
	{
		DebugRenderResult::RenderLine renderLine;
		renderLine.color = this->debugColor;
		renderLine.line = edge;
		renderResult->AddRenderLine(renderLine);
	}
}

/*virtual*/ bool BoxShape::RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const
{
	const Transform& worldToObject = this->GetWorldToObjectTransform();
	Ray objectSpaceRay = worldToObject.TransformRay(ray);

	AxisAlignedBoundingBox objectSpaceBox;
	this->GetAxisAlignedBox(objectSpaceBox);
	if (!objectSpaceRay.CastAgainst(objectSpaceBox, alpha))
		return false;

	Vector3 hitPoint = objectSpaceRay.CalculatePoint(alpha);
	double tolerance = 1e-5;

	std::vector<Vector3> normalArray;
	this->GetCornerPointArray(normalArray, false);

	normalArray.push_back(Vector3(objectSpaceBox.minCorner.x, objectSpaceBox.minCorner.y, 0.0));
	normalArray.push_back(Vector3(objectSpaceBox.maxCorner.x, objectSpaceBox.minCorner.y, 0.0));
	normalArray.push_back(Vector3(objectSpaceBox.minCorner.x, objectSpaceBox.maxCorner.y, 0.0));
	normalArray.push_back(Vector3(objectSpaceBox.maxCorner.x, objectSpaceBox.maxCorner.y, 0.0));

	normalArray.push_back(Vector3(objectSpaceBox.minCorner.x, 0.0, objectSpaceBox.minCorner.z));
	normalArray.push_back(Vector3(objectSpaceBox.maxCorner.x, 0.0, objectSpaceBox.minCorner.z));
	normalArray.push_back(Vector3(objectSpaceBox.minCorner.x, 0.0, objectSpaceBox.maxCorner.z));
	normalArray.push_back(Vector3(objectSpaceBox.maxCorner.x, 0.0, objectSpaceBox.maxCorner.z));

	normalArray.push_back(Vector3(0.0, objectSpaceBox.minCorner.y, objectSpaceBox.minCorner.z));
	normalArray.push_back(Vector3(0.0, objectSpaceBox.maxCorner.y, objectSpaceBox.minCorner.z));
	normalArray.push_back(Vector3(0.0, objectSpaceBox.minCorner.y, objectSpaceBox.maxCorner.z));
	normalArray.push_back(Vector3(0.0, objectSpaceBox.maxCorner.y, objectSpaceBox.maxCorner.z));

	normalArray.push_back(Vector3(objectSpaceBox.minCorner.x, 0.0, 0.0));
	normalArray.push_back(Vector3(objectSpaceBox.maxCorner.x, 0.0, 0.0));

	normalArray.push_back(Vector3(0.0, objectSpaceBox.minCorner.y, 0.0));
	normalArray.push_back(Vector3(0.0, objectSpaceBox.maxCorner.y, 0.0));

	normalArray.push_back(Vector3(0.0, 0.0, objectSpaceBox.minCorner.z));
	normalArray.push_back(Vector3(0.0, 0.0, objectSpaceBox.maxCorner.z));

	for (int i = 0; i < (signed)normalArray.size(); i++)
	{
		Vector3 mask;
		
		if (i < 8)
			mask = Vector3(1.0, 1.0, 1.0);
		else if (i < 12)
			mask = Vector3(1.0, 1.0, 0.0);
		else if (i < 16)
			mask = Vector3(1.0, 0.0, 1.0);
		else if (i < 20)
			mask = Vector3(0.0, 1.0, 1.0);
		else if (i < 22)
			mask = Vector3(1.0, 0.0, 0.0);
		else if (i < 24)
			mask = Vector3(0.0, 1.0, 0.0);
		else
			mask = Vector3(0.0, 0.0, 1.0);

		const Vector3& normal = normalArray[i];
		if ((hitPoint * mask).IsPoint(normal, tolerance))
		{
			unitSurfaceNormal = this->objectToWorld.TransformNormal(normal).Normalized();
			return true;
		}
	}

	COLL_SYS_ASSERT(false);
	return false;
}