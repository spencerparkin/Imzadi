#include "Polygon.h"
#include "Math/Plane.h"
#include "Math/Ray.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/LineSegment.h"
#include "Math/Matrix3x3.h"
#include "Math/Interval.h"
#include "Collision/Result.h"
#include <list>

using namespace Imzadi;

//----------------------------- PolygonShape -----------------------------

PolygonShape::PolygonShape(const PolygonShape& polygon) : Shape(true)
{
	for (const Vector3& vertex : polygon.vertexArray)
		this->vertexArray.push_back(vertex);
}

PolygonShape::PolygonShape(bool temporary) : Shape(temporary)
{
}

/*virtual*/ PolygonShape::~PolygonShape()
{
}

/*static*/ PolygonShape* PolygonShape::Create()
{
	return new PolygonShape(false);
}

/*virtual*/ ShapeCache* PolygonShape::CreateCache() const
{
	return new PolygonShapeCache();
}

/*virtual*/ Shape::TypeID PolygonShape::GetShapeTypeID() const
{
	return TypeID::POLYGON;
}

/*static*/ Shape::TypeID PolygonShape::StaticTypeID()
{
	return TypeID::POLYGON;
}

/*virtual*/ Shape* PolygonShape::Clone() const
{
	auto polygon = PolygonShape::Create();
	polygon->Copy(this);
	return polygon;
}

/*virtual*/ bool PolygonShape::Copy(const Shape* shape)
{
	if (!Shape::Copy(shape))
		return false;

	auto polygon = shape->Cast<PolygonShape>();
	if (!polygon)
		return false;

	this->vertexArray.clear();
	for (const Vector3& vertex : polygon->vertexArray)
		this->vertexArray.push_back(vertex);

	return true;
}

/*virtual*/ bool PolygonShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (this->vertexArray.size() < 3)
		return false;

	for (const Vector3& vertex : this->vertexArray)
		if (!vertex.IsValid())
			return false;

	// Make sure that all the points are coplanar.
	constexpr double tolerance = 1e-4;
	const Plane& plane = this->GetPlane();
	for (const Vector3& vertex : this->vertexArray)
	{
		double distance = plane.SignedDistanceTo(vertex);
		if (::fabs(distance) >= tolerance)
			return false;
	}

	// Make sure the polygon is convex.
	for (int i = 0; i < (signed)this->vertexArray.size(); i++)
	{
		const Vector3& vertexA = this->vertexArray[i];
		const Vector3& vertexB = this->vertexArray[(i + 1) % this->vertexArray.size()];
		Plane edgePlane(vertexA, (vertexB - vertexA).Cross(plane.unitNormal).Normalized());
		for (int j = 0; j < (signed)this->vertexArray.size(); j++)
		{
			double distance = edgePlane.SignedDistanceTo(vertexArray[j]);
			if (distance >= tolerance)
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

	for (int i = 0; i < (signed)this->vertexArray.size(); i++)
	{
		int j = (i + 1) % this->vertexArray.size();

		const Vector3& vertexA = this->vertexArray[i];
		const Vector3& vertexB = this->vertexArray[j];

		area += (vertexA - center).Cross(vertexB - center).Length() / 2.0;
	}

	return area;
}

/*virtual*/ bool PolygonShape::Split(const Plane& plane, Shape*& shapeBack, Shape*& shapeFront) const
{
	constexpr double planeThickness = 1e-6;

	auto polygonBack = new PolygonShape(false);
	auto polygonFront = new PolygonShape(false);

	for (int i = 0; i < (signed)this->vertexArray.size(); i++)
	{
		int j = (i + 1) % this->vertexArray.size();

		const Vector3& vertexA = this->vertexArray[i];
		const Vector3& vertexB = this->vertexArray[j];

		Plane::Side side = plane.GetSide(vertexA, planeThickness);

		if (side == Plane::Side::BACK || side == Plane::Side::NEITHER)
			polygonBack->vertexArray.push_back(vertexA);
		if (side == Plane::Side::FRONT || side == Plane::Side::NEITHER)
			polygonFront->vertexArray.push_back(vertexA);

		double edgeLength = 0.0;
		Ray ray(vertexA, vertexB - vertexA);
		ray.unitDirection.Normalize(&edgeLength);
		double alpha = 0.0;
		Interval interval(0.0, edgeLength);

		if (ray.CastAgainst(plane, alpha) && interval.ContainsInteriorValue(alpha))
		{
			Vector3 intersectionPoint = ray.CalculatePoint(alpha);
			polygonBack->vertexArray.push_back(intersectionPoint);
			polygonFront->vertexArray.push_back(intersectionPoint);
		}
	}

	if (polygonBack->vertexArray.size() < 3 || polygonFront->vertexArray.size() < 3)
	{
		delete polygonBack;
		delete polygonFront;
		return false;
	}

	shapeBack = polygonBack;
	shapeFront = polygonFront;
	return true;
}

const std::vector<Vector3>& PolygonShape::GetWorldVertices() const
{
	return ((PolygonShapeCache*)this->GetCache())->worldVertexArray;
}

/*virtual*/ bool PolygonShape::ContainsPoint(const Vector3& point) const
{
	// Is the point on the plane of the polygon?
	const Plane& worldPlane = this->GetWorldPlane();
	double tolerance = 1e-5;
	if (worldPlane.GetSide(point, tolerance) != Plane::Side::NEITHER)
		return false;

	std::vector<LineSegment> edgeArray;
	this->GetWorldEdges(edgeArray);

	// Is the point on an edge of the polygon?
	for (const LineSegment& edgeSegment : edgeArray)
		if (edgeSegment.ShortestDistanceTo(point) < tolerance)
			return true;

	// Is the point an interior point of the polygon?
	for(const LineSegment& edgeSegment : edgeArray)
	{
		const Vector3& vertexA = edgeSegment.point[0];
		const Vector3& vertexB = edgeSegment.point[1];

		double determinant = (vertexA - point).Cross(vertexB - point).Dot(worldPlane.unitNormal);
		if (determinant < 0.0)
			return false;
	}

	return true;
}

/*virtual*/ void PolygonShape::DebugRender(DebugRenderResult* renderResult) const
{
	DebugRenderResult::RenderLine renderLine;
	renderLine.color = this->debugColor;

	std::vector<LineSegment> edgeArray;
	this->GetWorldEdges(edgeArray);
	for(const LineSegment& edge : edgeArray)
	{
		renderLine.line = edge;
		renderResult->AddRenderLine(renderLine);
	}

	const Plane& worldPlane = this->GetWorldPlane();
	const Vector3& worldCenter = this->GetWorldCenter();

	renderLine.line.point[0] = worldCenter;
	renderLine.line.point[1] = worldCenter + worldPlane.unitNormal;
	renderResult->AddRenderLine(renderLine);
}

/*virtual*/ bool PolygonShape::RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const
{
	if (this->ContainsPoint(ray.origin))
		return false;

	const Plane& worldPlane = this->GetWorldPlane();
	if (!ray.CastAgainst(worldPlane, alpha) || alpha < 0.0)
		return false;

	Vector3 hitPoint = ray.CalculatePoint(alpha);
	if (!this->ContainsPoint(hitPoint))
		return false;

	unitSurfaceNormal = worldPlane.unitNormal;
	if (unitSurfaceNormal.Dot(ray.unitDirection) > 0.0)
		unitSurfaceNormal = -unitSurfaceNormal;

	return true;
}

void PolygonShape::Clear()
{
	this->vertexArray.clear();
}

void PolygonShape::AddVertex(const Vector3& point)
{
	this->vertexArray.push_back(point);
}

void PolygonShape::SetNumVertices(uint32_t vertexCount)
{
	this->vertexArray.resize(vertexCount);
}

void PolygonShape::SetVertex(int i, const Vector3& point)
{
	this->vertexArray[this->ModIndex(i)] = point;
}

const Vector3& PolygonShape::GetVertex(int i) const
{
	return this->vertexArray[this->ModIndex(i)];
}

const Plane& PolygonShape::GetPlane() const
{
	return ((PolygonShapeCache*)this->GetCache())->plane;
}

const Plane& PolygonShape::GetWorldPlane() const
{
	return ((PolygonShapeCache*)this->GetCache())->worldPlane;
}

const Vector3& PolygonShape::GetCenter() const
{
	return ((PolygonShapeCache*)this->GetCache())->center;
}

const Vector3& PolygonShape::GetWorldCenter() const
{
	return ((PolygonShapeCache*)this->GetCache())->worldCenter;
}

int PolygonShape::ModIndex(int i) const
{
	int j = i % this->vertexArray.size();
	if (j < 0)
		j += this->vertexArray.size();
	return j;
}

// TODO: I think this may have a bug in it.
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
	
	for (const Vector3& point : this->vertexArray)
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

	Matrix3x3 matrixXY, matrixXZ, matrixYZ;

	// z = ax + by + c
	matrixXY.ele[0][0] = sum_xx;
	matrixXY.ele[0][1] = sum_xy;
	matrixXY.ele[0][1] = sum_x;
	matrixXY.ele[1][0] = sum_xy;
	matrixXY.ele[1][1] = sum_yy;
	matrixXY.ele[1][2] = sum_y;
	matrixXY.ele[2][0] = sum_x;
	matrixXY.ele[2][1] = sum_y;
	matrixXY.ele[2][2] = double(this->vertexArray.size());

	// y = ax + bz + c
	matrixXZ.ele[0][0] = sum_xx;
	matrixXZ.ele[0][1] = sum_xz;
	matrixXZ.ele[0][2] = sum_x;
	matrixXZ.ele[1][0] = sum_xz;
	matrixXZ.ele[1][1] = sum_zz;
	matrixXZ.ele[1][2] = sum_z;
	matrixXZ.ele[2][0] = sum_x;
	matrixXZ.ele[2][1] = sum_z;
	matrixXZ.ele[2][2] = double(this->vertexArray.size());

	// x = ay + bz + c
	matrixYZ.ele[0][0] = sum_yy;
	matrixYZ.ele[0][1] = sum_yz;
	matrixYZ.ele[0][2] = sum_y;
	matrixYZ.ele[1][0] = sum_yz;
	matrixYZ.ele[1][1] = sum_zz;
	matrixYZ.ele[1][2] = sum_z;
	matrixYZ.ele[2][0] = sum_y;
	matrixYZ.ele[2][1] = sum_z;
	matrixYZ.ele[2][2] = double(this->vertexArray.size());

	double volXY = ::abs(matrixXY.Determinant());
	double volXZ = ::abs(matrixXZ.Determinant());
	double volYZ = ::abs(matrixYZ.Determinant());

	double a, b, c;
	Vector3 normal;

	if (volXY >= volXZ && volXY >= volYZ)
	{
		Matrix3x3 matrixInv;
		if (!matrixInv.Invert(matrixXY))
			return false;
		
		Vector3 coeficients = matrixInv * Vector3(sum_xz, sum_yz, sum_z);

		a = coeficients.x;
		b = coeficients.y;
		c = coeficients.z;

		normal = Vector3(a, b, -1.0);
	}
	else if (volXZ >= volXY && volXZ >= volYZ)
	{
		Matrix3x3 matrixInv;
		if (!matrixInv.Invert(matrixXZ))
			return false;

		Vector3 coeficients = matrixInv * Vector3(sum_xy, sum_yz, sum_y);

		a = coeficients.x;
		b = coeficients.y;
		c = coeficients.z;

		normal = Vector3(a, -1.0, b);
	}
	else //if (volYZ > volXY && volYZ > volXZ)
	{
		Matrix3x3 matrixInv;
		if (!matrixInv.Invert(matrixYZ))
			return false;

		Vector3 coeficients = matrixInv * Vector3(sum_xy, sum_xz, sum_x);

		a = coeficients.x;
		b = coeficients.y;
		c = coeficients.z;

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
	for (Vector3& vertex : this->vertexArray)
	{
		double distance = plane.SignedDistanceTo(vertex);
		vertex -= plane.unitNormal * distance;
	}
}

void PolygonShape::SetAsConvexHull(const std::vector<Vector3>& pointCloud)
{
	PolygonShape polygon(true);
	for (const Vector3& point : pointCloud)
		polygon.AddVertex(point);

	Plane plane;
	polygon.CalculatePlaneOfBestFit(plane);
	polygon.SnapToPlane(plane);

	// This can be expensive, and there is an algorithm with better time-complexity, but do this for now.
	// We don't want there to be any redundant points in the list.
	std::vector<Vector3> planarPointCloud;
	for (const Vector3& vertex : polygon.vertexArray)
		if (vertex.IsAnyPoint(planarPointCloud, 1e-5))
			planarPointCloud.push_back(vertex);

	this->CalculateConvexHullInternal(planarPointCloud, plane);
}

void PolygonShape::FixWindingOfTriangle(const Vector3& desiredNormal)
{
	if (this->vertexArray.size() == 3)
	{
		const Vector3& pointA = this->vertexArray[0];
		const Vector3& pointB = this->vertexArray[1];
		const Vector3& pointC = this->vertexArray[2];

		double determinant = (pointB - pointA).Cross(pointC - pointA).Dot(desiredNormal);
		if (determinant < 0.0)
		{
			this->vertexArray[0] = pointA;
			this->vertexArray[1] = pointC;
			this->vertexArray[2] = pointB;
		}
	}
}

// TODO: Test this.  It has not yet been tested, and so it surely doesn't work yet.
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

	IMZADI_ASSERT(planarPointCloudA.size() > 0);
	IMZADI_ASSERT(planarPointCloudB.size() > 0);

	// Divided, go conquer.
	PolygonShape polygonA(true), polygonB(true);
	polygonA.CalculateConvexHullInternal(planarPointCloudA, plane);
	polygonB.CalculateConvexHullInternal(planarPointCloudB, plane);

	// The secret sauce is now the ability to simply combine two convex hulls into one.
	for (const Vector3& vertex : polygonA.vertexArray)
		this->AddVertex(vertex);
	for (const Vector3& vertex : polygonB.vertexArray)
		this->AddVertex(vertex);

	// Handle some trivial cases first.
	if (polygonA.vertexArray.size() + polygonB.vertexArray.size() <= 3)
	{
		this->FixWindingOfTriangle(plane.unitNormal);
		return;
	}

	// Make a combined list of all edges concerned.  Note that the order of edges here is important.
	// It doesn't matter that we started with polygonA, then did polygonB.  What matters is that
	// generally, the edges go in a counter-clock-wise direction.
	std::list<LineSegment> edgeList;
	for (int i = 0; i < (signed)polygonA.vertexArray.size(); i++)
		edgeList.push_back(LineSegment(polygonA.GetVertex(i), polygonA.GetVertex(i + 1)));
	for (int i = 0; i < (signed)polygonB.vertexArray.size(); i++)
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

		if (edgePlane.AnyPointOnSide(this->vertexArray, Plane::Side::BACK))
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

void PolygonShape::GetWorldEdges(std::vector<LineSegment>& edgeArray) const
{
	edgeArray.clear();
	const std::vector<Vector3>& worldVertexArray = this->GetWorldVertices();
	for (int i = 0; i < (signed)worldVertexArray.size(); i++)
	{
		int j = (i + 1) % worldVertexArray.size();

		const Vector3& pointA = worldVertexArray[i];
		const Vector3& pointB = worldVertexArray[j];

		edgeArray.push_back(LineSegment(pointA, pointB));
	}
}

Vector3 PolygonShape::ClosestPointTo(const Vector3& point) const
{
	const Plane& worldPlane = this->GetWorldPlane();
	Vector3 closestPoint = worldPlane.ClosestPointTo(point);
	if (this->ContainsPoint(closestPoint))
		return closestPoint;

	double smallestDistance = std::numeric_limits<double>::max();
	std::vector<LineSegment> edgeArray;
	this->GetWorldEdges(edgeArray);
	for (const LineSegment& edge : edgeArray)
	{
		Vector3 edgePoint = edge.ClosestPointTo(point);
		double distance = (point - edgePoint).Length();
		if (distance < smallestDistance)
		{
			smallestDistance = distance;
			closestPoint = edgePoint;
		}
	}

	return closestPoint;
}

bool PolygonShape::IntersectsWith(const LineSegment& lineSegment, Vector3& intersectionPoint) const
{
	Vector3 unitDirection = lineSegment.GetDelta();
	double length = 0.0;
	if (!unitDirection.Normalize(&length))
		return false;

	Ray ray(lineSegment.point[0], unitDirection);
	const Plane& worldPlane = this->GetWorldPlane();
	double alpha = 0.0;
	if (!ray.CastAgainst(worldPlane, alpha))
		return false;

	if (alpha < 0.0 || alpha > length)
		return false;

	intersectionPoint = ray.CalculatePoint(alpha);
	return this->ContainsPoint(intersectionPoint);
}

/*virtual*/ bool PolygonShape::Dump(std::ostream& stream) const
{
	if (!Shape::Dump(stream))
		return false;

	stream << uint32_t(this->vertexArray.size());
	for (const Vector3& vertex : this->vertexArray)
		vertex.Dump(stream);

	return true;
}

/*virtual*/ bool PolygonShape::Restore(std::istream& stream)
{
	if (!Shape::Restore(stream))
		return false;

	uint32_t numVertices = 0;
	stream >> numVertices;
	this->vertexArray.clear();
	for (uint32_t i = 0; i < numVertices; i++)
	{
		Vector3 vertex;
		vertex.Restore(stream);
		this->vertexArray.push_back(vertex);
	}

	return true;
}

//----------------------------- PolygonShapeCache -----------------------------

PolygonShapeCache::PolygonShapeCache()
{
}

/*virtual*/ PolygonShapeCache::~PolygonShapeCache()
{
}

/*virtual*/ void PolygonShapeCache::Update(const Shape* shape)
{
	ShapeCache::Update(shape);

	auto polygon = (const PolygonShape*)shape;
	
	if (polygon->vertexArray.size() <= 2)
	{
		this->plane = Plane();
		this->center = Vector3(0.0, 0.0, 0.0);
	}
	else
	{
		this->center = Vector3(0.0, 0.0, 0.0);
		for (const Vector3& vertex : polygon->vertexArray)
			this->center += vertex;
		
		this->center /= double(polygon->vertexArray.size());
		this->worldCenter = polygon->objectToWorld.TransformPoint(this->center);

		Vector3 normal(0.0, 0.0, 0.0);

		for (int i = 0; i < (signed)polygon->vertexArray.size(); i++)
		{
			int j = (i + 1) % polygon->vertexArray.size();

			const Vector3& vertexA = polygon->vertexArray[i];
			const Vector3& vertexB = polygon->vertexArray[j];

			normal += (vertexA - this->center).Cross(vertexB - this->center);
		}

		this->plane = Plane(center, normal.Normalized());
		this->worldPlane = polygon->objectToWorld.TransformPlane(this->plane);
	}

	this->worldVertexArray.clear();
	for (const Vector3& vertex : polygon->vertexArray)
		this->worldVertexArray.push_back(polygon->objectToWorld.TransformPoint(vertex));

	this->boundingBox.SetToBoundPointCloud(this->worldVertexArray);
}