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
		Collision::Vector3 eyePoint;
		Collision::Vector3 lookAtPoint;
		Collision::Vector3 upVector;
	};

	void GetViewingParameters(ViewingParameters& viewingParams) const;

	const Collision::Transform& GetCameraToWorldTransform() const { return this->cameraToWorld; }

	const Collision::Vector3& GetCameraPosition() const;
	void SetCameraPosition(const Collision::Vector3& position);

	void GetCameraFrame(Collision::Vector3& xAxis, Collision::Vector3& yAxis, Collision::Vector3& zAxis) const;

	Collision::Quaternion GetCameraOrientation() const;
	void SetCameraOrientation(const Collision::Quaternion& quat);

	void SetCameraTarget(const Collision::Vector3& target);

private:
	Collision::Transform cameraToWorld;
};