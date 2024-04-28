#include "Polygon.h"
#include "Math/Plane.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/LineSegment.h"
#include "Math/Matrix3x3.h"
#include "Result.h"
#include <list>

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

/*virtual*/ void PolygonShape::RecalculateCache() const
{
	Shape::RecalculateCache();

	if (this->vertexArray->size() > 0)
	{
		this->cache.boundingBox.minCorner = this->objectToWorld.TransformPoint((*this->vertexArray)[0]);
		for (int i = 1; i < (signed)this->vertexArray->size(); i++)
			this->cache.boundingBox.Expand(this->objectToWorld.TransformPoint((*this->vertexArray)[i]));
	}
}

/*virtual*/ bool PolygonShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (this->vertexArray->size() < 3)
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

	// Make sure we have non-zero area.
	if (this->CalcSize() == 0.0)
		return false;

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
	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		int j = (i + 1) % this->vertexArray->size();

		DebugRenderResult::RenderLine renderLine;

		renderLine.line.point[0] = this->objectToWorld.TransformPoint((*this->vertexArray)[i]);
		renderLine.line.point[1] = this->objectToWorld.TransformPoint((*this->vertexArray)[j]);
		renderLine.color = this->debugColor;

		renderResult->AddRenderLine(renderLine);
	}

	Vector3 center = this->objectToWorld.TransformPoint(this->GetCenter());

	for (int i = 0; i < (signed)this->vertexArray->size(); i++)
	{
		DebugRenderResult::RenderLine renderLine;

		renderLine.line.point[0] = center;
		renderLine.line.point[1] = this->objectToWorld.TransformPoint((*this->vertexArray)[i]);
		renderLine.color = this->debugColor;

		renderResult->AddRenderLine(renderLine);
	}
}

/*virtual*/ bool PolygonShape::RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const
{
	// TODO: Write this.
	return false;
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
	if (!this->cachedPlaneValid && this->vertexArray->size() >= 3)
	{
		this->CalculatePlaneOfBestFit(this->cachedPlane);

		Vector3 center = this->GetCenter();
		Vector3 frontDirection = ((*this->vertexArray)[0] - center).Cross((*this->vertexArray)[1] - (*this->vertexArray)[0]);
		if (frontDirection.Dot(cachedPlane.unitNormal) < 0.0)
			cachedPlane.unitNormal = -cachedPlane.unitNormal;

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

bool PolygonShape::CalculatePlaneOfBestFit(Plane& plane) const
{
	plane.center = Vector3(0.0, 0.0, 0.0);
	plane.unitNormal = Vector3(0.0, 0.0, 1.0);

	double sum_x = 0.0;
	double sum_y = 0.0;
	double sum_z = 0.0;
	double sum_xx = 0.0;
	double sum_xy = 0.0;
	double sum_xz = 0.0;
	double sum_yy = 0.0;
	double sum_yz = 0.0;
	double sum_zz = 0.0;
	
	for (const Vector3& point : *this->vertexArray)
	{
		sum_x += point.x;
		sum_y += point.y;
		sum_z += point.z;
		sum_xx += point.x * point.x;
		sum_xy += point.x * point.y;
		sum_xz += point.x * point.z;
		sum_yy += point.y * point.y;
		sum_yz += point.y * point.z;
		sum_zz += point.z * point.z;
	}

	// TODO: Maybe a better thing to do is create 3 different matrices,
	//       then check their determinants.  Use the one with the largest
	//       absolute value.

	Matrix3x3 matrix;

	matrix.ele[0][0] = sum_xx;
	matrix.ele[0][1] = sum_xy;
	matrix.ele[0][1] = sum_x;
	matrix.ele[1][0] = sum_xy;
	matrix.ele[1][1] = sum_yy;
	matrix.ele[1][2] = sum_y;
	matrix.ele[2][0] = sum_x;
	matrix.ele[2][1] = sum_y;
	matrix.ele[2][2] = double(this->vertexArray->size());

	Matrix3x3 matrixInv;
	Vector3 normal;
	double a, b, c;

	// Solve for the coeficients of the the equation: z = ax + by + c.

	if (matrixInv.Invert(matrix))
	{
		Vector3 coeficients = matrixInv * Vector3(sum_xz, sum_yz, sum_z);
		
		a = coeficients.x;
		b = coeficients.y;
		c = coeficients.z;

		// Our plane equation is: z = ax + by + c <==> 0 = ax + by - z + c.

		normal = Vector3(a, b, -1.0);
	}
	else
	{
		// Here, the norm of the plane has a z-component of zero.  So we'll try to solve for the coeficients of: x = ay + bz + c.

		matrix.ele[0][0] = sum_yy;
		matrix.ele[0][1] = sum_yz;
		matrix.ele[0][2] = sum_y;
		matrix.ele[1][0] = sum_yz;
		matrix.ele[1][1] = sum_zz;
		matrix.ele[1][2] = sum_z;
		matrix.ele[2][0] = sum_y;
		matrix.ele[2][1] = sum_z;
		matrix.ele[2][2] = double(this->vertexArray->size());

		if (!matrix.Invert(matrix))
			return false;

		Vector3 coeficients = matrixInv * Vector3(sum_xy, sum_xz, sum_x);

		a = coeficients.x;
		b = coeficients.y;
		c = coeficients.z;

		// Our plane equation is: x = ay + bz + c <==> 0 = -x + ay + bz + c.

		normal = Vector3(-1.0, a, b);
	}

	double length = 0.0;
	if (!normal.Normalize(&length))
		return false;

	plane.unitNormal = normal;
	plane.center = -normal * c / length;

	return true;
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

	Plane plane;
	polygon.CalculatePlaneOfBestFit(plane);
	polygon.SnapToPlane(plane);

	// This can be expensive, and there is an algorithm with better time-complexity, but do this for now.
	// We don't want there to be any redundant points in the list.
	std::vector<Vector3> planarPointCloud;
	for (const Vector3& vertex : *polygon.vertexArray)
		if (vertex.IsAnyPoint(planarPointCloud, 1e-5))
			planarPointCloud.push_back(vertex);

	this->CalculateConvexHullInternal(planarPointCloud, plane);
}

void PolygonShape::FixWindingOfTriangle(const Vector3& desiredNormal)
{
	if (this->vertexArray->size() == 3)
	{
		const Vector3& pointA = (*this->vertexArray)[0];
		const Vector3& pointB = (*this->vertexArray)[1];
		const Vector3& pointC = (*this->vertexArray)[2];

		double determinant = (pointB - pointA).Cross(pointC - pointA).Dot(desiredNormal);
		if (determinant < 0.0)
		{
			(*this->vertexArray)[0] = pointA;
			(*this->vertexArray)[1] = pointC;
			(*this->vertexArray)[2] = pointB;
		}
	}
}

void PolygonShape::CalculateConvexHullInternal(const std::vector<Vector3>& planarPointCloud, const Plane& plane)
{
	this->Clear();

	// Handle the trivial cases first.  If they don't apply, then we divide and conquer.
	if (planarPointCloud.size() <= 3)
	{
		for (const Vector3& point : planarPointCloud)
			this->AddVertex(point);

		this->FixWindingOfTriangle(plane.unitNormal);
		return;
	}
	
	// Find the center of the planar point-cloud.
	Vector3 center(0.0, 0.0, 0.0);
	for (const Vector3& point : planarPointCloud)
		center += point;
	center /= double(planarPointCloud.size());

	// Find a point furthest from the center.
	double largestDistance = 0.0;
	Vector3 furthestPoint;
	for (const Vector3& point : planarPointCloud)
	{
		double distance = (point - center).Length();
		if (distance > largestDistance)
		{
			largestDistance = distance;
			furthestPoint = point;
		}
	}

	// I'm guessing this might be a good dividing plane.
	Plane dividingPlane;
	dividingPlane.center = center;
	dividingPlane.unitNormal = (furthestPoint - center).Normalized();

	// Break the planar-point-cloud into two smaller such sets.
	std::vector<Vector3> planarPointCloudA, planarPointCloudB;
	for (const Vector3& point : planarPointCloud)
	{
		double signedDistance = dividingPlane.SignedDistanceTo(point);
		if (signedDistance < 0.0)
			planarPointCloudA.push_back(point);
		else
			planarPointCloudB.push_back(point);
	}

	COLL_SYS_ASSERT(planarPointCloudA.size() > 0);
	COLL_SYS_ASSERT(planarPointCloudB.size() > 0);

	// Divided, go conquer.
	PolygonShape polygonA, polygonB;
	polygonA.CalculateConvexHullInternal(planarPointCloudA, plane);
	polygonB.CalculateConvexHullInternal(planarPointCloudB, plane);

	// The secret sauce is now the ability to simply combine two convex hulls into one.
	for (const Vector3& vertex : *polygonA.vertexArray)
		this->AddVertex(vertex);
	for (const Vector3& vertex : *polygonB.vertexArray)
		this->AddVertex(vertex);

	// Handle some trivial cases first.
	if (polygonA.vertexArray->size() + polygonB.vertexArray->size() <= 3)
	{
		this->FixWindingOfTriangle(plane.unitNormal);
		return;
	}

	// Make a combined list of all edges concerned.  Note that the order of edges here is important.
	// It doesn't matter that we started with polygonA, then did polygonB.  What matters is that
	// generally, the edges go in a counter-clock-wise direction.
	std::list<LineSegment> edgeList;
	for (int i = 0; i < (signed)polygonA.vertexArray->size(); i++)
		edgeList.push_back(LineSegment(polygonA.GetVertex(i), polygonA.GetVertex(i + 1)));
	for (int i = 0; i < (signed)polygonB.vertexArray->size(); i++)
		edgeList.push_back(LineSegment(polygonB.GetVertex(i), polygonB.GetVertex(i + 1)));

	// Cull those that are not on the convex hull.
	std::list<LineSegment>::iterator iter = edgeList.begin();
	while (iter != edgeList.end())
	{
		std::list<LineSegment>::iterator nextIter(iter);
		nextIter++;

		const LineSegment& edge = *iter;

		Plane edgePlane;
		edgePlane.center = edge.point[0];
		edgePlane.unitNormal = (edge.point[1] - edge.point[0]).Cross(plane.unitNormal).Normalized();

		if (edgePlane.AnyPointOnSide(*this->vertexArray, Plane::Side::BACK))
			edgeList.erase(iter);

		iter = nextIter;
	}

	// Wipe the polygon; we're about to construct it.
	this->Clear();

	// Walk edges, making two new ones in the process.
	iter = edgeList.begin();
	while (iter != edgeList.end())
	{
		std::list<LineSegment>::iterator nextIter(iter);
		nextIter++;

		const LineSegment& edge = *iter;
		this->AddVertex(edge.point[0]);

		if (nextIter == edgeList.end())
			break;

		const LineSegment& nextEdge = *nextIter;
		if (!edge.point[1].IsPoint(nextEdge.point[0]))
			this->AddVertex(edge.point[1]);

		iter = nextIter;
	}
}