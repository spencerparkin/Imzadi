#include "AnimTransform.h"
#include "Transform.h"

using namespace Imzadi;

AnimTransform::AnimTransform()
{
	this->SetIdentity();
}

AnimTransform::AnimTransform(const AnimTransform& transform)
{
	this->scale = transform.scale;
	this->rotation = transform.rotation;
	this->translation = transform.translation;
}

AnimTransform::AnimTransform(const Vector3& scale, const Quaternion& rotation, const Vector3& translation)
{
	this->scale = scale;
	this->rotation = rotation;
	this->translation = translation;
}

/*virtual*/ AnimTransform::~AnimTransform()
{
}

void AnimTransform::operator=(const AnimTransform& transform)
{
	this->scale = transform.scale;
	this->rotation = transform.rotation;
	this->translation = transform.translation;
}

bool AnimTransform::IsValid() const
{
	return this->scale.IsValid() && this->rotation.IsValid() && this->translation.IsValid();
}

void AnimTransform::SetIdentity()
{
	this->scale.SetComponents(1.0, 1.0, 1.0);
	this->rotation.SetIdentity();
	this->translation.SetComponents(0.0, 0.0, 0.0);
}

bool AnimTransform::SetFromTransform(const Transform& transform)
{
	Matrix3x3 rotationMat, shearMat, scaleMat;
	if (!transform.matrix.FactorRHS(rotationMat, shearMat, scaleMat))
		return false;

	double shearA = shearMat.ele[0][1];
	double shearB = shearMat.ele[0][2];
	double shearC = shearMat.ele[1][2];
	double tolerance = 1e-5;
	if (::fabs(shearA) >= tolerance || ::fabs(shearB) >= tolerance || ::fabs(shearC) >= tolerance)
		return false;

	this->scale.SetComponents(scaleMat.ele[0][0], scaleMat.ele[1][1], scaleMat.ele[2][2]);
	rotationMat.GetToQuat(this->rotation);
	this->translation = transform.translation;

	return true;
}

bool AnimTransform::GetToTransform(Transform& transform) const
{
	Matrix3x3 rotationMat, scaleMat;

	rotationMat.SetFromQuat(this->rotation);
	scaleMat.SetIdentity();
	scaleMat.ele[0][0] = this->scale.x;
	scaleMat.ele[1][1] = this->scale.y;
	scaleMat.ele[2][2] = this->scale.z;

	transform.translation = this->translation;
	transform.matrix = rotationMat * scaleMat;

	return true;
}

Vector3 AnimTransform::TransformPoint(const Vector3& point) const
{
	return this->rotation.Rotate(point * this->scale) + this->translation;
}

Vector3 AnimTransform::TransformVector(const Vector3& vector) const
{
	return this->rotation.Rotate(vector * this->scale);
}

void AnimTransform::Interpolate(const AnimTransform& transformA, const AnimTransform& transformB, double alpha)
{
	this->scale.Lerp(transformA.scale, transformB.scale, alpha);
	this->rotation.Interpolate(transformA.rotation, transformB.rotation, alpha);
	this->translation.Lerp(transformA.translation, transformB.translation, alpha);
}

bool AnimTransform::Concatinate(const AnimTransform& transformA, const AnimTransform& transformB)
{
	return this->SetFromTransform(transformB * transformA);
}

void AnimTransform::Dump(std::ostream& stream) const
{
	this->scale.Dump(stream);
	this->rotation.Dump(stream);
	this->translation.Dump(stream);
}

void AnimTransform::Restore(std::istream& stream)
{
	this->scale.Restore(stream);
	this->rotation.Restore(stream);
	this->translation.Restore(stream);
}

namespace Imzadi
{
	Transform operator*(const AnimTransform& transformA, const AnimTransform& transformB)
	{
		Transform regularTransformA, regularTransformB;
		transformA.GetToTransform(regularTransformA);
		transformB.GetToTransform(regularTransformB);
		return regularTransformA * regularTransformB;
	}
}