#include "Frustum.h"
#include "Matrix4x4.h"
#include "Plane.h"
#include "AxisAlignedBoundingBox.h"
#include "Transform.h"
#include "Ray.h"
#include <math.h>

using namespace Imzadi;

Frustum::Frustum()
{
	this->vfovi = M_PI / 3.0;
	this->hfovi = M_PI / 3.0;
	this->nearClip = 0.1;
	this->farClip = 1000.0;
}

Frustum::Frustum(const Frustum& frustum)
{
	this->vfovi = frustum.vfovi;
	this->hfovi = frustum.hfovi;
	this->nearClip = frustum.nearClip;
	this->farClip = frustum.farClip;
}

/*virtual*/ Frustum::~Frustum()
{
}

void Frustum::operator=(const Frustum& frustum)
{
	this->vfovi = frustum.vfovi;
	this->hfovi = frustum.hfovi;
	this->nearClip = frustum.nearClip;
	this->farClip = frustum.farClip;
}

void Frustum::GetPlanes(std::vector<Plane>& planeArray) const
{
	planeArray.push_back(Plane(Vector3(0.0, 0.0, -this->nearClip), Vector3(0.0, 0.0, 1.0)));
	planeArray.push_back(Plane(Vector3(0.0, 0.0, -this->farClip), Vector3(0.0, 0.0, -1.0)));

	double cosPhi = ::cos(this->hfovi / 2.0);
	double sinPhi = ::sin(this->hfovi / 2.0);

	planeArray.push_back(Plane(Vector3(0.0, 0.0, 0.0), Vector3(cosPhi, 0.0, sinPhi)));
	planeArray.push_back(Plane(Vector3(0.0, 0.0, 0.0), Vector3(-cosPhi, 0.0, sinPhi)));

	cosPhi = ::cos(this->vfovi / 2.0);
	sinPhi = ::sin(this->vfovi / 2.0);

	planeArray.push_back(Plane(Vector3(0.0, 0.0, 0.0), Vector3(0.0, cosPhi, sinPhi)));
	planeArray.push_back(Plane(Vector3(0.0, 0.0, 0.0), Vector3(0.0, -cosPhi, sinPhi)));
}

bool Frustum::IntersectedBySphere(const Vector3& center, double radius) const
{
	std::vector<Plane> planeArray;
	this->GetPlanes(planeArray);

	for (const Plane& plane : planeArray)
	{
		double signedDistance = plane.SignedDistanceTo(center);
		if (signedDistance >= radius)
			return false;
	}

	return true;
}

void Frustum::GetToProjectionMatrix(Matrix4x4& matrix) const
{
	matrix.SetIdentity();
	matrix.ele[0][0] = 1.0 / tan(this->hfovi / 2.0);
	matrix.ele[1][1] = 1.0 / tan(this->vfovi / 2.0);
	matrix.ele[2][2] = -this->farClip / (this->farClip - this->nearClip);
	matrix.ele[3][3] = 0.0;
	matrix.ele[2][3] = -this->farClip * this->nearClip / (this->farClip - this->nearClip);
	matrix.ele[3][2] = -1.0;
}

bool Frustum::SetFromProjectionMatrix(const Matrix4x4& matrix)
{
	// TODO: Write this.
	return false;
}

void Frustum::SetFromAspectRatio(double aspectRatio, double hfovi, double nearClip, double farClip)
{
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->hfovi = hfovi;
	this->vfovi = 2.0 * atan(tan(hfovi / 2.0) / aspectRatio);
}