#include "Camera.h"
#include "Scene.h"

using namespace Collision;

Camera::Camera()
{
	this->worldToCameraValid = false;
}

/*virtual*/ Camera::~Camera()
{
}

bool Camera::IsApproximatelyVisible(const RenderObject* renderObject) const
{
	Vector3 center;
	double radius = 0.0;
	renderObject->GetWorldBoundingSphere(center, radius);
	return this->frustum.IntersectedBySphere(center, radius);
}

void Camera::SetCameraToWorldTransform(const Collision::Transform& cameraToWorld)
{
	this->cameraToWorld = cameraToWorld;
	this->worldToCameraValid = false;
}

const Collision::Transform& Camera::GetWorldToCameraTransform() const
{
	if (!this->worldToCameraValid)
	{
		this->worldToCamera.Invert(this->cameraToWorld);
		this->worldToCameraValid = true;
	}

	return this->worldToCamera;
}

bool Camera::LookAt(const Collision::Vector3& eyePoint, const Collision::Vector3& focalPoint, const Collision::Vector3& upVector)
{
	this->cameraToWorld.translation = eyePoint;

	Vector3 xAxis, yAxis, zAxis;

	zAxis = eyePoint - focalPoint;
	if (!zAxis.Normalize())
		return false;

	xAxis = upVector.Cross(zAxis);
	if (!xAxis.Normalize())
		return false;

	yAxis = zAxis.Cross(xAxis);

	this->cameraToWorld.matrix.SetColumnVectors(xAxis, yAxis, zAxis);
	return true;
}