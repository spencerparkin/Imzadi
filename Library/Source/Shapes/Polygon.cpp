#include "Polygon.h"
#include "Math/Plane.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Matrix3x3.h"

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

	// TODO: Write this.
}