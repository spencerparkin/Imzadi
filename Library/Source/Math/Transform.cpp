#include "Transform.h"
#include "Plane.h"
#include "Ray.h"

using namespace Collision;

Transform::Transform()
{
	this->SetIdentity();
}

Transform::Transform(const Vector3& unitAxis, double angle, const Vector3& translation)
{
	this->matrix.SetFromAxisAngle(unitAxis, angle);
	this->translation = translation;
}

Transform::Transform(const Matrix3x3& matrix, const Vector3& translation)
{
	this->matrix = matrix;
	this->translation = translation;
}

Transform::Transform(const Transform& transform)
{
	this->matrix = transform.matrix;
	this->translation = translation;
}

/*virtual*/ Transform::~Transform()
{
}

bool Transform::IsValid() const
{
	if (!this->matrix.IsValid())
		return false;

	if (!this->translation.IsValid())
		return false;

	return true;
}

void Transform::SetIdentity()
{
	this->matrix.SetIdentity();
	this->translation = Vector3(0.0, 0.0, 0.0);
}

Vector3 Transform::TransformPoint(const Vector3& point) const
{
	return this->matrix * point + this->translation;
}


Vector3 Transform::TransformNormal(const Vector3& normal) const
{
	return this->matrix * normal;
}

Plane Transform::TransformPlane(const Plane& plane) const
{
	Plane transformedPlane;
	transformedPlane.center = this->TransformPoint(plane.center);
	transformedPlane.unitNormal = this->TransformNormal(plane.unitNormal).Normalized();
	return transformedPlane;
}

Ray Transform::TransformRay(const Ray& ray) const
{
	Ray transformedRay;
	transformedRay.origin = this->TransformPoint(ray.origin);
	transformedRay.unitDirection = this->TransformNormal(ray.unitDirection).Normalized();
	return transformedRay;
}

Transform Transform::Inverted() const
{
	Transform inverse;
	inverse.Invert(*this);
	return inverse;
}

bool Transform::Invert(const Transform& transform)
{
	if (!this->matrix.Invert(transform.matrix))
		return false;

	this->translation = this->matrix * -transform.translation;
	return true;
}

namespace Collision
{
	Transform operator*(const Transform& transformA, const Transform& transformB)
	{
		Transform product;
		product.matrix = transformA.matrix * transformB.matrix;
		product.translation = transformA.matrix * transformB.translation + transformA.translation;
		return product;
	}
}