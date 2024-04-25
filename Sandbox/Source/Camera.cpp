#include "Camera.h"

using namespace Collision;

Camera::Camera()
{
	this->cameraToWorld.SetIdentity();
}

/*virtual*/ Camera::~Camera()
{
}

void Camera::GetViewingParameters(ViewingParameters& viewingParams) const
{
	Vector3 xAxis, yAxis, zAxis;
	this->cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

	viewingParams.eyePoint = this->cameraToWorld.translation;
	viewingParams.lookAtPoint = viewingParams.eyePoint - zAxis;
	viewingParams.upVector = yAxis;
}

const Collision::Vector3& Camera::GetCameraPosition() const
{
	return this->cameraToWorld.translation;
}

void Camera::SetCameraPosition(const Collision::Vector3& position)
{
	this->cameraToWorld.translation = position;
}

void Camera::SetCameraTarget(const Collision::Vector3& target)
{
	Vector3 xAxis, yAxis, zAxis;

	zAxis = (this->cameraToWorld.translation - target).Normalized();
	xAxis = Vector3(0.0, 1.0, 0.0).Cross(zAxis).Normalized();
	yAxis = zAxis.Cross(xAxis);

	this->cameraToWorld.matrix.SetColumnVectors(xAxis, yAxis, zAxis);
}

void Camera::GetCameraFrame(Collision::Vector3& xAxis, Collision::Vector3& yAxis, Collision::Vector3& zAxis) const
{
	this->cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);
}

Collision::Quaternion Camera::GetCameraOrientation() const
{
	Quaternion quat;
	this->cameraToWorld.matrix.GetToQuat(quat);
	return quat;
}

void Camera::SetCameraOrientation(const Collision::Quaternion& quat)
{
	this->cameraToWorld.matrix.SetFromQuat(quat);
}