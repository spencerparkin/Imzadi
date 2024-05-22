#include "Camera.h"
#include "Scene.h"
#include "Math/Matrix4x4.h"

using namespace Collision;

Camera::Camera()
{
	this->worldToCameraValid = false;
	this->viewMode = ViewMode::PERSPECTIVE;
	this->orthoParams.width = 20.0;
	this->orthoParams.height = 20.0;
	this->orthoParams.nearClip = 0.0;
	this->orthoParams.farClip = 1000.0;
	this->orthoParams.desiredAspectRatio = 0.0;
	this->orthoParams.adjustWidth = true;
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

void Camera::GetProjectionMatrix(Collision::Matrix4x4& matrix) const
{
	switch (this->viewMode)
	{
		case ViewMode::PERSPECTIVE:
		{
			this->frustum.GetToProjectionMatrix(matrix);
			break;
		}
		case ViewMode::ORTHOGRAPHIC:
		{
			double width = this->orthoParams.width;
			double height = this->orthoParams.height;

			if (this->orthoParams.desiredAspectRatio != 0.0)
			{
				if (this->orthoParams.adjustWidth)
					width += height * this->orthoParams.desiredAspectRatio - width;
				else
					height += width / this->orthoParams.desiredAspectRatio - height;
			}

			matrix.SetIdentity();
			matrix.ele[0][0] = 2.0 / width;
			matrix.ele[1][1] = 2.0 / height;
			matrix.ele[2][2] = -1.0 / (this->orthoParams.farClip - this->orthoParams.nearClip);
			matrix.ele[2][3] = -this->orthoParams.nearClip / (this->orthoParams.farClip - this->orthoParams.nearClip);
			break;
		}
		default:
		{
			matrix.SetIdentity();
			break;
		}
	}
}