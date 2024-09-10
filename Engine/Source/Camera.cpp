#include "Camera.h"
#include "Scene.h"
#include "Math/Matrix4x4.h"

using namespace Imzadi;

Camera::Camera()
{
	this->worldToCameraValid = false;
	this->viewMode = ViewMode::PERSPECTIVE;
	this->orthoParams.width = 20.0;
	this->orthoParams.height = 20.0;
	this->orthoParams.nearClip = 0.0;
	this->orthoParams.farClip = 10000.0;
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
	if (!renderObject->GetWorldBoundingSphere(center, radius))
		return true;

	switch (this->viewMode)
	{
		case ViewMode::ORTHOGRAPHIC:
		{
			// TODO: Handle this case later.
			return true;
		}
		case ViewMode::PERSPECTIVE:
		{
			center = this->GetWorldToCameraTransform().TransformPoint(center);
			return this->frustum.IntersectedBySphere(center, radius);
		}
	}

	return true;
}

void Camera::SetCameraToWorldTransform(const Imzadi::Transform& cameraToWorld)
{
	this->cameraToWorld = cameraToWorld;
	this->worldToCameraValid = false;
}

const Transform& Camera::GetWorldToCameraTransform() const
{
	if (!this->worldToCameraValid)
	{
		this->worldToCamera.Invert(this->cameraToWorld);
		this->worldToCameraValid = true;
	}

	return this->worldToCamera;
}

bool Camera::LookAt(const Vector3& eyePoint, const Vector3& focalPoint, const Vector3& upVector)
{
	if (!this->cameraToWorld.LookAt(eyePoint, focalPoint, upVector))
		return false;

	this->worldToCameraValid = false;
	return true;
}

void Camera::GetProjectionMatrix(Matrix4x4& matrix) const
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