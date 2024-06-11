#pragma once

#include "Math/Transform.h"
#include "Math/Quaternion.h"

class Camera
{
public:
	Camera();
	virtual ~Camera();

	struct ViewingParameters
	{
		Imzadi::Vector3 eyePoint;
		Imzadi::Vector3 lookAtPoint;
		Imzadi::Vector3 upVector;
	};

	void GetViewingParameters(ViewingParameters& viewingParams) const;

	const Imzadi::Transform& GetCameraToWorldTransform() const { return this->cameraToWorld; }

	const Imzadi::Vector3& GetCameraPosition() const;
	void SetCameraPosition(const Imzadi::Vector3& position);

	void GetCameraFrame(Imzadi::Vector3& xAxis, Imzadi::Vector3& yAxis, Imzadi::Vector3& zAxis) const;

	Imzadi::Quaternion GetCameraOrientation() const;
	void SetCameraOrientation(const Imzadi::Quaternion& quat);

	void SetCameraTarget(const Imzadi::Vector3& target);

private:
	Imzadi::Transform cameraToWorld;
};