#include "AxisAlignedBoundingBox.h"
#include "LineSegment.h"
#include "Plane.h"

using namespace Imzadi;

AxisAlignedBoundingBox::AxisAlignedBoundingBox()
{
	this->minCorner.SetComponents(0.0, 0.0, 0.0);
	this->maxCorner.SetComponents(0.0, 0.0, 0.0);
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const Vector3& point)
{
	this->minCorner = point;
	this->maxCorner = point;
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb)
{
	this->minCorner = aabb.minCorner;
	this->maxCorner = aabb.maxCorner;
}

/*virtual*/ AxisAlignedBoundingBox::~AxisAlignedBoundingBox()
{
}

void AxisAlignedBoundingBox::operator=(const AxisAlignedBoundingBox& aabb)
{
	this->minCorner = aabb.minCorner;
	this->maxCorner = aabb.maxCorner;
}

bool AxisAlignedBoundingBox::IsValid() const
{
	if (!this->minCorner.IsValid())
		return false;

	if (!this->maxCorner.IsValid())
		return false;

	if (this->minCorner.x > this->maxCorner.x)
		return false;

	if (this->minCorner.y > this->maxCorner.y)
		return false;

	if (this->minCorner.z > this->maxCorner.z)
		return false;

	return true;
}

bool AxisAlignedBoundingBox::ContainsPoint(const Vector3& point, double borderThickness /*= 0.0*/) const
{
	if (!(-borderThickness / 2.0 + this->minCorner.x <= point.x && point.x <= this->maxCorner.x + borderThickness / 2.0))
		return false;

	if (!(-borderThickness / 2.0 + this->minCorner.y <= point.y && point.y <= this->maxCorner.y + borderThickness / 2.0))
		return false;

	if (!(-borderThickness / 2.0 + this->minCorner.z <= point.z && point.z <= this->maxCorner.z + borderThickness / 2.0))
		return false;

	return true;
}

bool AxisAlignedBoundingBox::ContainsPointOnSurface(const Vector3& point, double borderThickness /*= 0.0*/) const
{
	if (!ContainsPoint(point, borderThickness))
		return false;

	return this->PointOnFacePlane(point, borderThickness);
}

bool AxisAlignedBoundingBox::ContainsInteriorPoint(const Vector3& point, double borderThickness /*= 0.0*/) const
{
	if (!ContainsPoint(point, borderThickness))
		return false;

	return !this->PointOnFacePlane(point, borderThickness);
}

bool AxisAlignedBoundingBox::PointOnFacePlane(const Vector3& point, double borderThickness /*= 0.0*/) const
{
	if ((-borderThickness / 2.0 + this->minCorner.x <= point.x && point.x <= this->minCorner.x + borderThickness / 2.0) ||
		(-borderThickness / 2.0 + this->maxCorner.x <= point.x && point.x <= this->maxCorner.x + borderThickness / 2.0))
	{
		return true;
	}

	if ((-borderThickness / 2.0 + this->minCorner.y <= point.y && point.y <= this->minCorner.y + borderThickness / 2.0) ||
		(-borderThickness / 2.0 + this->maxCorner.y <= point.y && point.y <= this->maxCorner.y + borderThickness / 2.0))
	{
		return true;
	}

	if ((-borderThickness / 2.0 + this->minCorner.z <= point.z && point.z <= this->minCorner.z + borderThickness / 2.0) ||
		(-borderThickness / 2.0 + this->maxCorner.z <= point.z && point.z <= this->maxCorner.z + borderThickness / 2.0))
	{
		return true;
	}

	return false;
}

bool AxisAlignedBoundingBox::ContainsBox(const AxisAlignedBoundingBox& box) const
{
	return this->ContainsPoint(box.minCorner) && this->ContainsPoint(box.maxCorner);
}

bool AxisAlignedBoundingBox::Intersect(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB)
{
	this->minCorner.x = IMZADI_MAX(aabbA.minCorner.x, aabbB.minCorner.x);
	this->minCorner.y = IMZADI_MAX(aabbA.minCorner.y, aabbB.minCorner.y);
	this->minCorner.z = IMZADI_MAX(aabbA.minCorner.z, aabbB.minCorner.z);

	this->maxCorner.x = IMZADI_MIN(aabbA.maxCorner.x, aabbB.maxCorner.x);
	this->maxCorner.y = IMZADI_MIN(aabbA.maxCorner.y, aabbB.maxCorner.y);
	this->maxCorner.z = IMZADI_MIN(aabbA.maxCorner.z, aabbB.maxCorner.z);

	return this->IsValid();
}

void AxisAlignedBoundingBox::Scale(double scale)
{
	this->Scale(scale, scale, scale);
}

void AxisAlignedBoundingBox::Scale(double scaleX, double scaleY, double scaleZ)
{
	Vector3 center = this->GetCenter();
	Vector3 vector = this->maxCorner - center;
	vector.x *= scaleX;
	vector.y *= scaleY;
	vector.z *= scaleZ;
	this->minCorner = center - vector;
	this->maxCorner = center + vector;
}

Vector3 AxisAlignedBoundingBox::GetCenter() const
{
	return (this->minCorner + this->maxCorner) / 2.0;
}

void AxisAlignedBoundingBox::MakeReadyForExpansion()
{
	this->minCorner.x = std::numeric_limits<double>::max();
	this->minCorner.y = std::numeric_limits<double>::max();
	this->minCorner.z = std::numeric_limits<double>::max();

	this->maxCorner.x = -std::numeric_limits<double>::max();
	this->maxCorner.y = -std::numeric_limits<double>::max();
	this->maxCorner.z = -std::numeric_limits<double>::max();
}

void AxisAlignedBoundingBox::Expand(const Vector3& point)
{
	if (this->minCorner.x > point.x)
		this->minCorner.x = point.x;

	if (this->maxCorner.x < point.x)
		this->maxCorner.x = point.x;

	if (this->minCorner.y > point.y)
		this->minCorner.y = point.y;

	if (this->maxCorner.y < point.y)
		this->maxCorner.y = point.y;

	if (this->minCorner.z > point.z)
		this->minCorner.z = point.z;

	if (this->maxCorner.z < point.z)
		this->maxCorner.z = point.z;
}

void AxisAlignedBoundingBox::Expand(const std::vector<Vector3>& pointArray)
{
	for (const Vector3& point : pointArray)
		this->Expand(point);
}

void AxisAlignedBoundingBox::Expand(const AxisAlignedBoundingBox& box)
{
	this->Expand(Vector3(box.minCorner.x, box.minCorner.y, box.minCorner.z));
	this->Expand(Vector3(box.maxCorner.x, box.minCorner.y, box.minCorner.z));
	this->Expand(Vector3(box.minCorner.x, box.maxCorner.y, box.minCorner.z));
	this->Expand(Vector3(box.maxCorner.x, box.maxCorner.y, box.minCorner.z));
	this->Expand(Vector3(box.minCorner.x, box.minCorner.y, box.maxCorner.z));
	this->Expand(Vector3(box.maxCorner.x, box.minCorner.y, box.maxCorner.z));
	this->Expand(Vector3(box.minCorner.x, box.maxCorner.y, box.maxCorner.z));
	this->Expand(Vector3(box.maxCorner.x, box.maxCorner.y, box.maxCorner.z));
}

void AxisAlignedBoundingBox::Split(AxisAlignedBoundingBox& aabbA, AxisAlignedBoundingBox& aabbB, Plane* divisionPlane /*= nullptr*/) const
{
	double xSize = 0.0, ySize = 0.0, zSize = 0.0;
	this->GetDimensions(xSize, ySize, zSize);

	aabbA = *this;
	aabbB = *this;

	if (xSize >= ySize && xSize >= zSize)
	{
		double middle = (this->minCorner.x + this->maxCorner.x) / 2.0;
		aabbA.maxCorner.x = middle;
		aabbB.minCorner.x = middle;

		if (divisionPlane)
			*divisionPlane = Plane(aabbA.maxCorner, Vector3(1.0, 0.0, 0.0));
	}
	else if (ySize >= xSize && ySize >= zSize)
	{
		double middle = (this->minCorner.y + this->maxCorner.y) / 2.0;
		aabbA.maxCorner.y = middle;
		aabbB.minCorner.y = middle;

		if (divisionPlane)
			*divisionPlane = Plane(aabbA.maxCorner, Vector3(0.0, 1.0, 0.0));
	}
	else //if(zSize >= xSize && zSize >= ySize)
	{
		double middle = (this->minCorner.z + this->maxCorner.z) / 2.0;
		aabbA.maxCorner.z = middle;
		aabbB.minCorner.z = middle;

		if (divisionPlane)
			*divisionPlane = Plane(aabbA.maxCorner, Vector3(0.0, 0.0, 1.0));
	}
}

void AxisAlignedBoundingBox::GetDimensions(double& xSize, double& ySize, double& zSize) const
{
	xSize = this->maxCorner.x - this->minCorner.x;
	ySize = this->maxCorner.y - this->minCorner.y;
	zSize = this->maxCorner.z - this->minCorner.z;
}

void AxisAlignedBoundingBox::SetToBoundPointCloud(const std::vector<Vector3>& pointCloud)
{
	if (pointCloud.size() == 0)
		return;

	this->minCorner = pointCloud[0];
	this->maxCorner = this->minCorner;

	for (int i = 1; i < (signed)pointCloud.size(); i++)
		this->Expand(pointCloud[i]);
}

void AxisAlignedBoundingBox::GetSidePlanes(std::vector<Plane>& sidePlaneArray) const
{
	double xSize = 0.0, ySize = 0.0, zSize = 0.0;
	this->GetDimensions(xSize, ySize, zSize);

	Plane plane;

	if (xSize > 0.0 && ySize > 0.0)
	{
		plane.unitNormal = Vector3(0.0, 0.0, -1.0);
		plane.center.SetComponents(this->minCorner.x, this->minCorner.y, this->minCorner.z);
		sidePlaneArray.push_back(plane);

		plane.unitNormal = Vector3(0.0, 0.0, 1.0);
		plane.center.SetComponents(this->minCorner.x, this->minCorner.y, this->maxCorner.z);
		sidePlaneArray.push_back(plane);
	}

	if (xSize > 0.0 && zSize > 0.0)
	{
		plane.unitNormal = Vector3(0.0, -1.0, 0.0);
		plane.center.SetComponents(this->minCorner.x, this->minCorner.y, this->minCorner.z);
		sidePlaneArray.push_back(plane);

		plane.unitNormal = Vector3(0.0, 1.0, 0.0);
		plane.center.SetComponents(this->minCorner.x, this->maxCorner.y, this->minCorner.z);
		sidePlaneArray.push_back(plane);
	}

	if (ySize > 0.0 && zSize > 0.0)
	{
		plane.unitNormal = Vector3(-1.0, 0.0, 0.0);
		plane.center.SetComponents(this->minCorner.x, this->minCorner.y, this->minCorner.z);
		sidePlaneArray.push_back(plane);

		plane.unitNormal = Vector3(1.0, 0.0, 0.0);
		plane.center.SetComponents(this->maxCorner.x, this->minCorner.y, this->minCorner.z);
		sidePlaneArray.push_back(plane);
	}
}

void AxisAlignedBoundingBox::GetVertices(std::vector<Vector3>& vertexArray) const
{
	vertexArray.push_back(Vector3(this->minCorner.x, this->minCorner.y, this->minCorner.z));
	vertexArray.push_back(Vector3(this->minCorner.x, this->minCorner.y, this->maxCorner.z));
	vertexArray.push_back(Vector3(this->minCorner.x, this->maxCorner.y, this->minCorner.z));
	vertexArray.push_back(Vector3(this->minCorner.x, this->maxCorner.y, this->maxCorner.z));
	vertexArray.push_back(Vector3(this->maxCorner.x, this->minCorner.y, this->minCorner.z));
	vertexArray.push_back(Vector3(this->maxCorner.x, this->minCorner.y, this->maxCorner.z));
	vertexArray.push_back(Vector3(this->maxCorner.x, this->maxCorner.y, this->minCorner.z));
	vertexArray.push_back(Vector3(this->maxCorner.x, this->maxCorner.y, this->maxCorner.z));
}

void AxisAlignedBoundingBox::GetEdgeSegments(std::vector<LineSegment>& edgeSegmentArray) const
{
	// -X to +X

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->minCorner.y, this->minCorner.z),
		Vector3(this->maxCorner.x, this->minCorner.y, this->minCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->maxCorner.y, this->minCorner.z),
		Vector3(this->maxCorner.x, this->maxCorner.y, this->minCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->minCorner.y, this->maxCorner.z),
		Vector3(this->maxCorner.x, this->minCorner.y, this->maxCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->maxCorner.y, this->maxCorner.z),
		Vector3(this->maxCorner.x, this->maxCorner.y, this->maxCorner.z))
	);

	// -Y to +Y

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->minCorner.y, this->minCorner.z),
		Vector3(this->minCorner.x, this->maxCorner.y, this->minCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->maxCorner.x, this->minCorner.y, this->minCorner.z),
		Vector3(this->maxCorner.x, this->maxCorner.y, this->minCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->minCorner.y, this->maxCorner.z),
		Vector3(this->minCorner.x, this->maxCorner.y, this->maxCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->maxCorner.x, this->minCorner.y, this->maxCorner.z),
		Vector3(this->maxCorner.x, this->maxCorner.y, this->maxCorner.z))
	);

	// -Z to +Z

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->minCorner.y, this->minCorner.z),
		Vector3(this->minCorner.x, this->minCorner.y, this->maxCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->maxCorner.x, this->minCorner.y, this->minCorner.z),
		Vector3(this->maxCorner.x, this->minCorner.y, this->maxCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->minCorner.x, this->maxCorner.y, this->minCorner.z),
		Vector3(this->minCorner.x, this->maxCorner.y, this->maxCorner.z))
	);

	edgeSegmentArray.push_back(LineSegment(
		Vector3(this->maxCorner.x, this->maxCorner.y, this->minCorner.z),
		Vector3(this->maxCorner.x, this->maxCorner.y, this->maxCorner.z))
	);
}

Vector3 AxisAlignedBoundingBox::ClosestPointTo(const Vector3& point) const
{
	Vector3 closestPoint;
	double smallestSquareDistance = std::numeric_limits<double>::max();

	std::vector<LineSegment> edgeSegmentArray;
	this->GetEdgeSegments(edgeSegmentArray);
	for (const LineSegment& edge : edgeSegmentArray)
	{
		Vector3 edgePoint = edge.ClosestPointTo(point);
		Vector3 delta = point - edgePoint;
		double squareDistance = delta.Dot(delta);
		if (squareDistance < smallestSquareDistance)
		{
			smallestSquareDistance = squareDistance;
			closestPoint = edgePoint;
		}
	}

	std::vector<Plane> sidePlaneArray;
	this->GetSidePlanes(sidePlaneArray);
	for (const Plane& sidePlane : sidePlaneArray)
	{
		Vector3 planePoint = sidePlane.ClosestPointTo(point);
		if (this->ContainsPoint(planePoint, 1e-5))
		{
			Vector3 delta = point - planePoint;
			double squareDistance = delta.Dot(delta);
			if (squareDistance < smallestSquareDistance)
			{
				smallestSquareDistance = squareDistance;
				closestPoint = planePoint;
			}
		}
	}

	return closestPoint;
}

double AxisAlignedBoundingBox::GetVolume() const
{
	double width = 0.0, height = 0.0, depth = 0.0;
	this->GetDimensions(width, height, depth);
	return width * height * depth;
}

void AxisAlignedBoundingBox::GetSphere(Vector3& center, double& radius) const
{
	// TODO: Write this.
}

void AxisAlignedBoundingBox::Dump(std::ostream& stream) const
{
	this->minCorner.Dump(stream);
	this->maxCorner.Dump(stream);
}

void AxisAlignedBoundingBox::Restore(std::istream& stream)
{
	this->minCorner.Restore(stream);
	this->maxCorner.Restore(stream);
}