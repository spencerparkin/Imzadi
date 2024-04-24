#include "Sphere.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Result.h"

using namespace Collision;

SphereShape::SphereShape()
{
	this->radius = 1.0;
}

/*virtual*/ SphereShape::~SphereShape()
{
}

/*static*/ SphereShape* SphereShape::Create()
{
	return new SphereShape();
}

/*virtual*/ Shape::TypeID SphereShape::GetShapeTypeID() const
{
	return TypeID::SPHERE;
}

/*virtual*/ void SphereShape::CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	Vector3 delta(this->radius, this->radius, this->radius);
	Vector3 worldCenter = this->objectToWorld.TransformPoint(this->center);

	boundingBox.minCorner = worldCenter + delta;
	boundingBox.maxCorner = worldCenter - delta;
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

/*virtual*/ void SphereShape::DebugRender(DebugRenderResult* renderResult) const
{
	const int latitudeCount = 8;
	const int longitudeCount = 16;

	Vector3 sphereVertices[latitudeCount][longitudeCount];

	for (int i = 0; i < latitudeCount; i++)
	{
		double latitudeAngle = M_PI * double(i) / double(latitudeCount - 1);

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
			renderLine.color.SetComponents(1.0, 1.0, 1.0);

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
			renderLine.color.SetComponents(1.0, 1.0, 1.0);

			renderResult->AddRenderLine(renderLine);
		}
	}
}