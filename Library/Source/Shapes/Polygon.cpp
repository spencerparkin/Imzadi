#include "Polygon.h"
#include "Math/Plane.h"
#include "Math/AxisAlignedBoundingBox.h"

using namespace Collision;

PolygonShape::PolygonShape()
{
	this->vertexArray = new std::vector<Vector3>();
	this->cachedPlaneValid = false;
}

/*virtual*/ PolygonShape::~PolygonShape()
{
	delete this->vertexArray;
}

/*static*/ PolygonShape* PolygonShape::Create()
{
	return new PolygonShape();
}

/*virtual*/ Shape::TypeID PolygonShape::GetShapeTypeID() const
{
	return TypeID::POLYGON;
}

/*virtual*/ void PolygonShape::CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	if (this->vertexArray->size() > 0)
	{
		boundingBox.minCorner = this->objectToWorld.TransformPoint((*this->vertexArray)[0]);
		for (int i = 1; i < (signed)this->vertexArray->size(); i++)
			boundingBox.Expand(this->objectToWorld.TransformPoint((*this->vertexArray)[i]));
	}
}

/*virtual*/ bool PolygonShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	for (const Vector3& vertex : *this->vertexArray)
		if (!vertex.IsValid())
			return false;

	// Make sure that all the points are coplanar.
	constexpr double tolerance = 1e-5;
	const Plane& plane = this->GetPlane();
	for (const Vector3& vertex : *this->vertexArray)
	{
		double distance = plane.SignedDistanceTo(vertex);
		if (::fabs(distance) >= tolerance)
			return false;
	}

	// Make sure the polygon is convex.
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[(i + 1) % this->vertexArray->size()];
		Plane edgePlane(vertexA, (vertexB - vertexA).Cross(plane.unitNormal).Normalized());
		for (int j = 0; j < (signed)this->vertexArray->size(); j++)
		{
			double distance = edgePlane.SignedDistanceTo((*vertexArray)[j]);
			if (distance > 0.0)
				return false;
		}
	}

	return true;
}

/*virtual*/ double PolygonShape::CalcSize() const
{
	double area = 0.0;
	Vector3 center = this->GetCenter();

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		const Vector3& vertexA = (*this->vertexArray)[i];
		const Vector3& vertexB = (*this->vertexArray)[(i + 1) % this->vertexArray->size()];

		area += (vertexA - center).Cross(vertexB - center).Length() / 2.0;
	}

	return area;
}

/*virtual*/ void PolygonShape::DebugRender(DebugRenderResult* renderResult) const
{
}

void PolygonShape::Clear()
{
	this->vertexArray->clear();
}

void PolygonShape::AddVertex(const Vector3& point)
{
	this->vertexArray->push_back(point);
}

void PolygonShape::SetNumVertices(uint32_t vertexCount)
{
	this->vertexArray->resize(vertexCount);
}

void PolygonShape::SetVertex(int i, const Vector3& point)
{
	(*this->vertexArray)[this->ModIndex(i)] = point;
}

const Vector3& PolygonShape::GetVertex(int i) const
{
	return (*this->vertexArray)[this->ModIndex(i)];
}

const Plane& PolygonShape::GetPlane() const
{
	if (!this->cachedPlaneValid)
	{
		this->cachedPlane = this->CalculatePlaneOfBestFit();
		this->cachedPlaneValid = true;
	}
	
	return this->cachedPlane;
}

Vector3 PolygonShape::GetCenter() const
{
	Vector3 center(0.0, 0.0, 0.0);

	if (this->vertexArray->size() > 0)
	{
		for (const Vector3& vertex : *this->vertexArray)
			center += vertex;

		center /= double(this->vertexArray->size());
	}

	return center;
}

int PolygonShape::ModIndex(int i) const
{
	int j = i % this->vertexArray->size();
	if (j < 0)
		j += this->vertexArray->size();
	return j;
}

Plane PolygonShape::CalculatePlaneOfBestFit() const
{
	Plane plane;

	// TODO: Write this.  Use linear least squares method.

	return plane;
}

void PolygonShape::SnapToPlane(const Plane& plane)
{
	for (Vector3& vertex : *this->vertexArray)
	{
		double distance = plane.SignedDistanceTo(vertex);
		vertex -= plane.unitNormal * distance;
	}
}

void PolygonShape::SetAsConvexHull(const std::vector<Vector3>& pointCloud)
{
	PolygonShape polygon;
	for (const Vector3& point : pointCloud)
		polygon.AddVertex(point);

	Plane plane = polygon.CalculatePlaneOfBestFit();
	polygon.SnapToPlane(plane);

	// TODO: Write this.  Perform Gram-Scan algorithm with plane and polygon.
}