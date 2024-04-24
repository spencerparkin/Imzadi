#include "Capsule.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Result.h"

using namespace Collision;

CapsuleShape::CapsuleShape()
{
	this->radius = 1.0;
}

/*virtual*/ CapsuleShape::~CapsuleShape()
{
}

/*static*/ CapsuleShape* CapsuleShape::Create()
{
	return new CapsuleShape();
}

/*virtual*/ Shape::TypeID CapsuleShape::GetShapeTypeID() const
{
	return TypeID::POLYGON;
}

/*virtual*/ void CapsuleShape::CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	Vector3 pointA = this->objectToWorld.TransformPoint(this->lineSegment.point[0]);
	Vector3 pointB = this->objectToWorld.TransformPoint(this->lineSegment.point[1]);

	boundingBox.minCorner = pointA;
	boundingBox.maxCorner = pointA;

	Vector3 delta(this->radius, this->radius, this->radius);

	boundingBox.Expand(pointA + delta);
	boundingBox.Expand(pointA - delta);
	boundingBox.Expand(pointB + delta);
	boundingBox.Expand(pointB - delta);
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
			vertex.z = cylinderLength * double(i) / double(numSlices);

			cylinderVertices[i][j] = renderTransform.TransformPoint(vertex);
		}
	}

	DebugRenderResult::RenderLine renderLine;

	renderLine.color.SetComponents(1.0, 1.0, 1.0);

	for (int i = 0; i < numSlices; i++)
	{
		for (int j = 0; j < numSegments; j++)
		{
			int k = (j + 1) % numSegments;

			renderLine.line.point[0] = cylinderVertices[i][j];
			renderLine.line.point[0] = cylinderVertices[i][k];

			renderResult->AddRenderLine(renderLine);
		}
	}

	const int numCapSlices = 5;
	Vector3 capVertices[numCapSlices][numSegments];

	for (int i = 0; i < numCapSlices; i++)
	{
		double latitudeAngle = (M_PI / 2.0) * double(i) / double(numSlices - 1);

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
				renderLine.line.point[0] = renderTransform.TransformPoint(capVertices[i][k]);

				renderResult->AddRenderLine(renderLine);
			}
		}

		for (int i = 0; i < numCapSlices - 1; i++)
		{
			for (int j = 0; j < numSegments; j++)
			{
				int k = i + 1;

				renderLine.line.point[0] = renderTransform.TransformPoint(capVertices[i][j]);
				renderLine.line.point[0] = renderTransform.TransformPoint(capVertices[k][j]);

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