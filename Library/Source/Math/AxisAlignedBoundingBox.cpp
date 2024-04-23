#include "AxisAlignedBoundingBox.h"

using namespace Collision;

AxisAlignedBoundingBox::AxisAlignedBoundingBox()
{
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

bool AxisAlignedBoundingBox::ContainsPoint(const Vector3& point) const
{
	if (!(this->minCorner.x <= point.x && point.x <= this->maxCorner.x))
		return false;

	if (!(this->minCorner.y <= point.y && point.y <= this->maxCorner.y))
		return false;

	if (!(this->minCorner.z <= point.z && point.z <= this->maxCorner.z))
		return false;

	return true;
}

bool AxisAlignedBoundingBox::Intersect(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB)
{
	// TODO: Write this.

	return false;
}

void AxisAlignedBoundingBox::Expand(const Vector3& point)
{
	if (this->minCorner.x < point.x)
		this->minCorner.x = point.x;

	if (this->maxCorner.x > point.x)
		this->maxCorner.x = point.x;

	if (this->minCorner.y < point.y)
		this->minCorner.y = point.y;

	if (this->maxCorner.y > point.y)
		this->maxCorner.y = point.y;

	if (this->minCorner.z < point.z)
		this->minCorner.z = point.z;

	if (this->maxCorner.z > point.z)
		this->maxCorner.z = point.z;
}

void AxisAlignedBoundingBox::Split(AxisAlignedBoundingBox& aabbA, AxisAlignedBoundingBox& aabbB) const
{
	double xSize = 0.0, ySize = 0.0, zSize = 0.0;
	this->GetDimensions(xSize, ySize, zSize);

	aabbA = *this;
	aabbB = *this;

	if (xSize > ySize && xSize > zSize)
	{
		double middle = (this->minCorner.x + this->maxCorner.x) / 2.0;
		aabbA.maxCorner.x = middle;
		aabbB.minCorner.x = middle;
	}
	else if (ySize > xSize && ySize > zSize)
	{
		double middle = (this->minCorner.y + this->maxCorner.y) / 2.0;
		aabbA.maxCorner.y = middle;
		aabbB.minCorner.y = middle;
	}
	else //if(zSize > xSize && zSize > xSize)
	{
		double middle = (this->minCorner.z + this->maxCorner.z) / 2.0;
		aabbA.maxCorner.z = middle;
		aabbB.minCorner.z = middle;
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