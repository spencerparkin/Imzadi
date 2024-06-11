#include "Sphere.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Quadratic.h"
#include "Math/Ray.h"
#include "Result.h"

using namespace Collision;

//------------------------------------- SphereShape -------------------------------------

SphereShape::SphereShape(bool temporary) : Shape(temporary)
{
	this->radius = 1.0;
}

/*virtual*/ SphereShape::~SphereShape()
{
}

/*static*/ SphereShape* SphereShape::Create()
{
	return new SphereShape(false);
}

/*virtual*/ ShapeCache* SphereShape::CreateCache() const
{
	return new SphereShapeCache();
}

/*virtual*/ Shape::TypeID SphereShape::GetShapeTypeID() const
{
	return TypeID::SPHERE;
}

/*static*/ Shape::TypeID SphereShape::StaticTypeID()
{
	return TypeID::SPHERE;
}

/*virtual*/ Shape* SphereShape::Clone() const
{
	auto sphere = SphereShape::Create();
	sphere->Copy(this);
	return sphere;
}

/*virtual*/ bool SphereShape::Copy(const Shape* shape)
{
	if (!Shape::Copy(shape))
		return false;

	auto sphere = shape->Cast<SphereShape>();
	if (!sphere)
		return false;

	this->radius = sphere->radius;
	this->center = sphere->center;
	return true;
}

/*virtual*/ bool SphereShape::IsValid() const
{
	if (!Shape::IsValid())
		return false;

	if (::isnan(this->radius) || ::isinf(this->radius))
		return false;

	if (!this->center.IsValid())
		return false;

	if (this->radius <= 0.0)
		return false;

	return true;
}

/*virtual*/ double SphereShape::CalcSize() const
{
	return (4.0 / 3.0) * M_PI * this->radius * this->radius * this->radius;
}

/*virtual*/ bool SphereShape::ContainsPoint(const Vector3& point) const
{
	return (point - this->objectToWorld.TransformPoint(this->center)).Length() <= this->radius;
}

/*virtual*/ void SphereShape::DebugRender(DebugRenderResult* renderResult) const
{
	const int latitudeCount = 8;
	const int longitudeCount = 16;

	Vector3 sphereVertices[latitudeCount][longitudeCount];

	for (int i = 0; i < latitudeCount; i++)
	{
		double latitudeAngle = -M_PI / 2.0 + M_PI * double(i) / double(latitudeCount - 1);

		for (int j = 0; j < longitudeCount; j++)
		{
			double longitudeAngle = 2.0 * M_PI * double(j) / double(longitudeCount);

			Vector3 longitudeAxis(::cos(longitudeAngle), ::sin(longitudeAngle), 0.0);
			Vector3 poleAxis(0.0, 0.0, 1.0);

			Vector3 vertex = this->radius * (::cos(latitudeAngle) * longitudeAxis + ::sin(latitudeAngle) * poleAxis);

			sphereVertices[i][j] = this->objectToWorld.TransformPoint(vertex);
		}
	}

	for (int i = 1; i < latitudeCount - 1; i++)
	{
		for (int j = 0; j < longitudeCount; j++)
		{
			int k = (j + 1) % longitudeCount;

			DebugRenderResult::RenderLine renderLine;

			renderLine.line.point[0] = sphereVertices[i][j];
			renderLine.line.point[1] = sphereVertices[i][k];
			renderLine.color = this->debugColor;

			renderResult->AddRenderLine(renderLine);
		}
	}

	for (int i = 0; i < longitudeCount; i++)
	{
		for (int j = 0; j < latitudeCount - 1; j++)
		{
			int k = j + 1;

			DebugRenderResult::RenderLine renderLine;

			renderLine.line.point[0] = sphereVertices[j][i];
			renderLine.line.point[1] = sphereVertices[k][i];
			renderLine.color = this->debugColor;

			renderResult->AddRenderLine(renderLine);
		}
	}
}

/*virtual*/ bool SphereShape::RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const
{
	Vector3 worldCenter = this->objectToWorld.TransformPoint(this->center);
	Vector3 delta = ray.origin - worldCenter;

	Quadratic quadratic;
	quadratic.A = 1.0;
	quadratic.B = 2.0 * delta.Dot(ray.unitDirection);
	quadratic.C = delta.Dot(delta) - this->radius * this->radius;

	std::vector<double> realRoots;
	quadratic.Solve(realRoots);

	// The ray misses the sphere.
	if (realRoots.size() == 0)
		return false;

	if (realRoots.size() == 1)
	{
		// The ray hits the sphere on a tangent.
		alpha = realRoots[0];
		if (alpha < 0.0)
			return false;	// The ray is pointing away from the sphere.
	}
	else if (realRoots.size() == 2)
	{
		// The ray enters and exits the sphere.
		if (realRoots[0] > 0.0 && realRoots[1] > 0.0)
			alpha = COLL_SYS_MIN(realRoots[0], realRoots[1]);
		else
		{
			// Here, the ray either originates within the sphere (which we
			// do not allow), or the line of the ray hits the sphere, but
			// the ray points away from the sphere.
			return false;
		}
	}

	Vector3 hitPoint = ray.CalculatePoint(alpha);
	unitSurfaceNormal = (hitPoint - worldCenter).Normalized();
	return true;
}

/*virtual*/ bool SphereShape::Dump(std::ostream& stream) const
{
	if (!Shape::Dump(stream))
		return false;

	stream << this->radius;
	this->center.Dump(stream);
	return true;
}

/*virtual*/ bool SphereShape::Restore(std::istream& stream)
{
	if (!Shape::Restore(stream))
		return false;

	stream >> this->radius;
	this->center.Restore(stream);
	return true;
}

//------------------------------------- SphereShapeCache -------------------------------------

SphereShapeCache::SphereShapeCache()
{
}

/*virtual*/ SphereShapeCache::~SphereShapeCache()
{
}

/*virtual*/ void SphereShapeCache::Update(const Shape* shape)
{
	ShapeCache::Update(shape);

	auto sphere = (const SphereShape*)shape;

	Vector3 delta(sphere->radius, sphere->radius, sphere->radius);
	Vector3 worldCenter = sphere->objectToWorld.TransformPoint(sphere->center);

	this->boundingBox.minCorner = worldCenter - delta;
	this->boundingBox.maxCorner = worldCenter + delta;
}