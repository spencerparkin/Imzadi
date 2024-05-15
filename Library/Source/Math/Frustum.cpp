#include "Frustum.h"
#include "Matrix4x4.h"
#include "Plane.h"
#include "AxisAlignedBoundingBox.h"
#include <math.h>

using namespace Collision;

Frustum::Frustum()
{
	this->vfovi = M_PI / 4.0;
	this->hfovi = M_PI / 4.0;
	this->near = 0.1;
	this->far = 1000.0;
}

Frustum::Frustum(const Frustum& frustum)
{
	this->vfovi = frustum.vfovi;
	this->hfovi = frustum.hfovi;
	this->near = frustum.near;
	this->far = frustum.far;
}

/*virtual*/ Frustum::~Frustum()
{
}

void Frustum::operator=(const Frustum& frustum)
{
	this->vfovi = frustum.vfovi;
	this->hfovi = frustum.hfovi;
	this->near = frustum.near;
	this->far = frustum.far;
}

void Frustum::GetPlanes(std::vector<Plane>& planeArray) const
{
	// TODO: Write this.
}

bool Frustum::IntersectedBy(const AxisAlignedBoundingBox& box) const
{
	// TODO: Write this.
	return false;
}

void Frustum::ToProjectionMatrix(Matrix4x4& matrix) const
{
	matrix.Identity();
	matrix.ele[0][0] = 1.0 / tan(this->hfovi / 2.0);
	matrix.ele[1][1] = 1.0 / tan(this->vfovi / 2.0);
	matrix.ele[2][2] = -this->far / (this->far - this->near);
	matrix.ele[3][3] = 0.0;
	matrix.ele[2][3] = this->far * this->near / (this->far - this->near);
	matrix.ele[3][2] = -1.0;
}

bool Frustum::FromProjectionMatrix(const Matrix4x4& matrix)
{
	// TODO: Write this.
	return false;
}