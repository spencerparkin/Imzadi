#include "Capsule.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Quadratic.h"
#include "Math/Ray.h"
#include "Sphere.h"
#include "Collision/Result.h"

using namespace Imzadi;
using namespace Imzadi::Collision;

//-------------------------------- CapsuleShape --------------------------------

CapsuleShape::CapsuleShape(bool temporary) : Shape(temporary)
{
	this->radius = 1.0;
}

/*virtual*/ CapsuleShape::~CapsuleShape()
{
}

/*static*/ CapsuleShape* CapsuleShape::Create()
{
	return new CapsuleShape(false);
}

/*virtual*/ ShapeCache* CapsuleShape::CreateCache() const
{
	return new CapsuleShapeCache();
}

/*virtual*/ Shape::TypeID CapsuleShape::GetShapeTypeID() const
{
	return TypeID::CAPSULE;
}

/*static*/ Shape::TypeID CapsuleShape::StaticTypeID()
{
	return TypeID::CAPSULE;
}

/*virtual*/ Shape* CapsuleShape::Clone() const
{
	auto capsule = CapsuleShape::Create();
	capsule->Copy(this);
	return capsule;
}

/*virtual*/ bool CapsuleShape::Copy(const Shape* shape)
{
	if (!Shape::Copy(shape))
		return false;

	auto capsule = shape->Cast<CapsuleShape>();
	if (!capsule)
		return false;

	this->lineSegment = capsule->lineSegment;
	this->radius = capsule->radius;
	return true;
}

/*virtual*/ bool CapsuleShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (::isnan(this->radius) || ::isinf(this->radius))
		return false;

	if (!this->lineSegment.point[0].IsValid() || !this->lineSegment.point[1].IsValid())
		return false;

	if (this->radius <= 0.0)
		return false;

	return true;
}

/*virtual*/ double CapsuleShape::CalcSize() const
{
	return(
		M_PI * this->radius * this->radius * this->lineSegment.Length() +
		(4.0 / 3.0) * M_PI * this->radius * this->radius * this->radius
	);
}

/*virtual*/ bool CapsuleShape::ContainsPoint(const Vector3& point) const
{
	LineSegment worldSegment = this->GetWorldToObjectTransform().TransformLineSegment(this->lineSegment);
	return worldSegment.ShortestDistanceTo(point) <= this->radius;
}

/*virtual*/ void CapsuleShape::DebugRender(DebugRenderResult* renderResult) const
{
	Transform axisAlignedToObject;
	axisAlignedToObject.translation = this->lineSegment.point[0];

	Vector3 xAxis, yAxis, zAxis;

	zAxis = (this->lineSegment.point[1] - this->lineSegment.point[0]).Normalized();
	yAxis.SetAsOrthogonalTo(zAxis);
	yAxis.Normalize();
	xAxis = yAxis.Cross(zAxis);

	axisAlignedToObject.matrix.SetColumnVectors(xAxis, yAxis, zAxis);

	Transform renderTransform = this->objectToWorld * axisAlignedToObject;

	const int numSlices = 10;
	const int numSegments = 20;

	Vector3 cylinderVertices[numSlices][numSegments];
	double cylinderLength = this->lineSegment.Length();

	for (int i = 0; i < numSlices; i++)
	{
		for (int j = 0; j < numSegments; j++)
		{
			double angle = 2.0 * M_PI * double(j) / double(numSegments);

			Vector3 vertex;

			vertex.x = this->radius * ::cos(angle);
			vertex.y = this->radius * ::sin(angle);
			vertex.z = cylinderLength * double(i) / double(numSlices - 1);

			cylinderVertices[i][j] = renderTransform.TransformPoint(vertex);
		}
	}

	DebugRenderResult::RenderLine renderLine;
	renderLine.color = this->debugColor;

	for (int i = 0; i < numSlices; i++)
	{
		for (int j = 0; j < numSegments; j++)
		{
			int k = (j + 1) % numSegments;

			renderLine.line.point[0] = cylinderVertices[i][j];
			renderLine.line.point[1] = cylinderVertices[i][k];

			renderResult->AddRenderLine(renderLine);
		}
	}

	for (int i = 0; i < numSegments; i++)
	{
		for (int j = 0; j < numSlices - 1; j++)
		{
			int k = j + 1;

			renderLine.line.point[0] = cylinderVertices[j][i];
			renderLine.line.point[1] = cylinderVertices[k][i];

			renderResult->AddRenderLine(renderLine);
		}
	}

	const int numCapSlices = 5;
	Vector3 capVertices[numCapSlices][numSegments];

	for (int i = 0; i < numCapSlices; i++)
	{
		double latitudeAngle = (M_PI / 2.0) * double(i) / double(numCapSlices - 1);

		for (int j = 0; j < numSegments; j++)
		{
			double longitudeAngle = 2.0 * M_PI * double(j) / double(numSegments);

			Vector3 longitudeAxis(::cos(longitudeAngle), ::sin(longitudeAngle), 0.0);
			Vector3 poleAxis(0.0, 0.0, 1.0);

			capVertices[i][j] = this->radius * (::cos(latitudeAngle) * longitudeAxis + ::sin(latitudeAngle) * poleAxis);
		}
	}

	auto renderCap = [&renderTransform, &capVertices, &renderLine, &renderResult]()
	{
		for (int i = 1; i < numCapSlices; i++)
		{
			for (int j = 0; j < numSegments; j++)
			{
				int k = (j + 1) % numSegments;

				renderLine.line.point[0] = renderTransform.TransformPoint(capVertices[i][j]);
				renderLine.line.point[1] = renderTransform.TransformPoint(capVertices[i][k]);

				renderResult->AddRenderLine(renderLine);
			}
		}

		for (int i = 0; i < numCapSlices - 1; i++)
		{
			for (int j = 0; j < numSegments; j++)
			{
				int k = i + 1;

				renderLine.line.point[0] = renderTransform.TransformPoint(capVertices[i][j]);
				renderLine.line.point[1] = renderTransform.TransformPoint(capVertices[k][j]);

				renderResult->AddRenderLine(renderLine);
			}
		}
	};

	Transform capToObject;

	capToObject.translation = this->lineSegment.point[0];
	capToObject.matrix.SetColumnVectors(xAxis, yAxis, -zAxis);
	renderTransform = this->objectToWorld * capToObject;
	renderCap();

	capToObject.translation = this->lineSegment.point[1];
	capToObject.matrix.SetColumnVectors(xAxis, yAxis, zAxis);
	renderTransform = this->objectToWorld * capToObject;
	renderCap();
}

/*virtual*/ bool CapsuleShape::RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const
{
	// Note that if we tried to take the ray to object space (rather than take the capsule to world space),
	// then we could handle object-to-world transforms with shear and/or non-uniform scale.  However, to
	// keep things simpler for now, I'm just going to do the calculations in world space and assume we don't
	// have any shear or non-uniform scale in the transform.

	LineSegment worldLineSegment = this->objectToWorld.TransformLineSegment(this->lineSegment);

	const Vector3& pointA = worldLineSegment.point[0];
	const Vector3& pointB = worldLineSegment.point[1];

	Vector3 unitVector = (pointB - pointA).Normalized();
	Vector3 delta = ray.origin - pointA;

	double dotA = ray.unitDirection.Dot(unitVector);
	double dotB = delta.Dot(ray.unitDirection);
	double dotC = delta.Dot(unitVector);
	double dotD = delta.Dot(delta);

	Quadratic tubeQuadratic;
	tubeQuadratic.A = 1.0 - dotA * dotA;
	tubeQuadratic.B = 2.0 * (dotB - dotC * dotA);
	tubeQuadratic.C = dotD - dotC * dotC - this->radius * this->radius;

	std::vector<double> tubeRoots;
	tubeQuadratic.Solve(tubeRoots);

	double tolerance = 1e-5;

	if ((tubeRoots.size() == 1 && tubeRoots[0] > 0.0) || (tubeRoots.size() == 2 && tubeRoots[0] > 0.0 && tubeRoots[1] > 0.0))
	{
		alpha = (tubeRoots.size() == 1) ? tubeRoots[0] : IMZADI_MIN(tubeRoots[0], tubeRoots[1]);
		Vector3 hitPoint = ray.CalculatePoint(alpha);
		double distance = worldLineSegment.ShortestDistanceTo(hitPoint);
		if (::fabs(distance - this->radius) < tolerance)
		{
			unitSurfaceNormal = (hitPoint - pointA - (hitPoint - pointA).ProjectedOnto(unitVector)).Normalized();
			return true;
		}
	}
	
	// If we get here, then we don't hite the cylindrical part of the capsule,
	// but we may hit one of the two hemisphere caps.  We can safely treat them
	// as full spheres, because we've eliminated the possibility of half of those
	// spheres getting hit by the ray.

	SphereShape capA(true);
	capA.SetCenter(pointA);
	capA.SetRadius(this->radius);
	if (capA.RayCast(ray, alpha, unitSurfaceNormal))
		return true;

	SphereShape capB(true);
	capB.SetCenter(pointB);
	capB.SetRadius(this->radius);
	if (capB.RayCast(ray, alpha, unitSurfaceNormal))
		return true;

	return false;
}

void CapsuleShape::SetVertex(int i, const Vector3& point)
{
	if (i % 2 == 0)
		this->lineSegment.point[0] = point;
	else
		this->lineSegment.point[1] = point;
}

const Vector3& CapsuleShape::GetVertex(int i) const
{
	return (i % 2 == 0) ? this->lineSegment.point[0] : this->lineSegment.point[1];
}

/*virtual*/ bool CapsuleShape::Dump(std::ostream& stream) const
{
	if (!Shape::Dump(stream))
		return false;

	stream << this->radius;
	this->lineSegment.Dump(stream);
	return true;
}

/*virtual*/ bool CapsuleShape::Restore(std::istream& stream)
{
	if (!Shape::Restore(stream))
		return false;

	stream >> this->radius;
	this->lineSegment.Restore(stream);
	return true;
}

//-------------------------------- CapsuleShapeCache --------------------------------

CapsuleShapeCache::CapsuleShapeCache()
{
}

/*virtual*/ CapsuleShapeCache::~CapsuleShapeCache()
{
}

/*virtual*/ void CapsuleShapeCache::Update(const Shape* shape)
{
	ShapeCache::Update(shape);

	auto capsule = (const CapsuleShape*)shape;

	Vector3 pointA = capsule->objectToWorld.TransformPoint(capsule->lineSegment.point[0]);
	Vector3 pointB = capsule->objectToWorld.TransformPoint(capsule->lineSegment.point[1]);

	this->boundingBox.minCorner = pointA;
	this->boundingBox.maxCorner = pointA;

	Vector3 delta(capsule->radius, capsule->radius, capsule->radius);

	this->boundingBox.Expand(pointA + delta);
	this->boundingBox.Expand(pointA - delta);
	this->boundingBox.Expand(pointB + delta);
	this->boundingBox.Expand(pointB - delta);
}