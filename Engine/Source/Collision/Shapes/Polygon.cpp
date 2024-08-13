#include "Polygon.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Collision/Result.h"
#include <list>

using namespace Imzadi;
using namespace Imzadi::Collision;

//----------------------------- PolygonShape -----------------------------

PolygonShape::PolygonShape(const PolygonShape& polygon) : Shape(true)
{
	this->localPolygon = polygon.localPolygon;
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

	this->localPolygon = polygon->localPolygon;
	return true;
}

/*virtual*/ bool PolygonShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (!this->localPolygon.IsValid())
		return false;

	if (!this->localPolygon.IsConvex())
		return false;

	// Make sure we have non-zero area.
	if (this->CalcSize() == 0.0)
		return false;

	return true;
}

/*virtual*/ double PolygonShape::CalcSize() const
{
	return this->localPolygon.Area();
}

/*virtual*/ bool PolygonShape::Split(const Plane& plane, Shape*& shapeBack, Shape*& shapeFront) const
{
	constexpr double planeThickness = 1e-6;

	auto polygonBack = new PolygonShape(false);
	auto polygonFront = new PolygonShape(false);

	if (!this->localPolygon.SplitAgainstPlane(plane, polygonBack->localPolygon, polygonFront->localPolygon, planeThickness))
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
	return ((PolygonShapeCache*)this->GetCache())->worldPolygon.vertexArray;
}

/*virtual*/ bool PolygonShape::ContainsPoint(const Vector3& point) const
{
	return ((PolygonShapeCache*)this->GetCache())->worldPolygon.ContainsPoint(point);
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
	return ((PolygonShapeCache*)this->GetCache())->worldPolygon.RayCast(ray, alpha, unitSurfaceNormal);
}

void PolygonShape::Clear()
{
	this->localPolygon.vertexArray.clear();
}

void PolygonShape::AddVertex(const Vector3& point)
{
	this->localPolygon.vertexArray.push_back(point);
}

void PolygonShape::SetNumVertices(uint32_t vertexCount)
{
	this->localPolygon.vertexArray.resize(vertexCount);
}

void PolygonShape::SetVertex(int i, const Vector3& point)
{
	this->localPolygon.vertexArray[this->ModIndex(i)] = point;
}

const Vector3& PolygonShape::GetVertex(int i) const
{
	return this->localPolygon.vertexArray[this->ModIndex(i)];
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
	return this->localPolygon.Mod(i);
}

void PolygonShape::GetWorldEdges(std::vector<LineSegment>& edgeArray) const
{
	((PolygonShapeCache*)this->GetCache())->worldPolygon.GetEdges(edgeArray);
}

Vector3 PolygonShape::ClosestPointTo(const Vector3& point) const
{
	return ((PolygonShapeCache*)this->GetCache())->worldPolygon.ClosestPointTo(point);
}

bool PolygonShape::IntersectsWith(const LineSegment& lineSegment, Vector3& intersectionPoint) const
{
	return ((PolygonShapeCache*)this->GetCache())->worldPolygon.IntersectsWith(lineSegment, intersectionPoint);
}

/*virtual*/ bool PolygonShape::Dump(std::ostream& stream) const
{
	if (!Shape::Dump(stream))
		return false;

	this->localPolygon.Dump(stream);
	return true;
}

/*virtual*/ bool PolygonShape::Restore(std::istream& stream)
{
	if (!Shape::Restore(stream))
		return false;

	this->localPolygon.Restore(stream);
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
	
	if (polygon->localPolygon.vertexArray.size() <= 2)
	{
		this->plane = Plane();
		this->center = Vector3(0.0, 0.0, 0.0);
	}
	else
	{
		this->center = polygon->localPolygon.CalcCenter();
		this->plane = polygon->localPolygon.CalcPlane(true);
	}

	this->worldCenter = polygon->objectToWorld.TransformPoint(this->center);
	this->worldPlane = polygon->objectToWorld.TransformPlane(this->plane);

	polygon->objectToWorld.TransformPolygon(polygon->localPolygon, this->worldPolygon);
	this->boundingBox.SetToBoundPointCloud(this->worldPolygon.vertexArray);
}