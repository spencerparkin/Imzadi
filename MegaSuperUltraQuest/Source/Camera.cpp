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
	const AxisAlignedBoundingBox& box = renderObject->GetWorldBoundingBox();

	Vector3 center;
	double radius;
	box.GetSphere(center, radius);

	center = this->GetWorldToCameraTransform().TransformPoint(center);

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