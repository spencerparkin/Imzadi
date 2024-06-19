#include "Transform.h"
#include "Plane.h"
#include "Ray.h"
#include "LineSegment.h"
#include "Matrix4x4.h"

using namespace Imzadi;

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
	this->translation = transform.translation;
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


Vector3 Transform::TransformVector(const Vector3& vector) const
{
	return this->matrix * vector;
}

LineSegment Transform::TransformLineSegment(const LineSegment& lineSegment) const
{
	return LineSegment(
		this->TransformPoint(lineSegment.point[0]),
		this->TransformPoint(lineSegment.point[1])
	);
}

Plane Transform::TransformPlane(const Plane& plane) const
{
	Plane transformedPlane;
	transformedPlane.center = this->TransformPoint(plane.center);
	transformedPlane.unitNormal = this->TransformVector(plane.unitNormal).Normalized();
	return transformedPlane;
}

Ray Transform::TransformRay(const Ray& ray) const
{
	Ray transformedRay;
	transformedRay.origin = this->TransformPoint(ray.origin);
	transformedRay.unitDirection = this->TransformVector(ray.unitDirection).Normalized();
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

void Transform::GetToMatrix(Matrix4x4& matrix) const
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			matrix.ele[i][j] = this->matrix.ele[i][j];

	matrix.ele[3][0] = 0.0;
	matrix.ele[3][1] = 0.0;
	matrix.ele[3][2] = 0.0;
	matrix.ele[3][3] = 1.0f;
	matrix.ele[0][3] = this->translation.x;
	matrix.ele[1][3] = this->translation.y;
	matrix.ele[2][3] = this->translation.z;
}

bool Transform::SetFromMatrix(const Matrix4x4& matrix)
{
	if (matrix.ele[3][0] != 0.0 || matrix.ele[3][1] != 0.0 || matrix.ele[3][2] != 0.0 || matrix.ele[3][3] != 1.0)
		return false;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->matrix.ele[i][j] = matrix.ele[i][j];

	this->translation.x = matrix.ele[0][3];
	this->translation.y = matrix.ele[1][3];
	this->translation.z = matrix.ele[2][3];
	return true;
}

void Transform::InterapolateBoneTransforms(const Transform& transformA, const Transform& transformB, double alpha)
{
	this->matrix.InterpolateOrientations(transformA.matrix, transformB.matrix, alpha);

	Vector3 vectorA = transformA.translation;
	Vector3 vectorB = transformB.translation;

	double lengthA = 0.0;
	vectorA.Normalize(&lengthA);

	double lengthB = 0.0;
	vectorB.Normalize(&lengthB);

	Vector3 vector;
	vector.Slerp(vectorA, vectorB, alpha);

	double length = lengthA + alpha * (lengthB - lengthA);

	this->translation = vector * length;
}

void Transform::Dump(std::ostream& stream) const
{
	this->matrix.Dump(stream);
	this->translation.Dump(stream);
}

void Transform::Restore(std::istream& stream)
{
	this->matrix.Restore(stream);
	this->translation.Restore(stream);
}

namespace Imzadi
{
	Transform operator*(const Transform& transformA, const Transform& transformB)
	{
		Transform product;
		product.matrix = transformA.matrix * transformB.matrix;
		product.translation = transformA.matrix * transformB.translation + transformA.translation;
		return product;
	}
}