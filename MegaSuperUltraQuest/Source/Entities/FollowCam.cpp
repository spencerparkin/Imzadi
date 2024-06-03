#include "FollowCam.h"
#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"
#include "Game.h"
#include "FreeCam.h"

using namespace Collision;

// TODO: Extra challenge: Adjust the radius of our camera's orbit so that no
//       collision object obstructs our view of the subject.  But don't really
//       change the orbit, just the effective orbit until the camera can
//       swivel out of the way.
FollowCam::FollowCam()
{
	this->followParams.maxRotationRate = M_PI / 3.0;
	this->followParams.objectSpaceFocalPoint.SetComponents(0.0, 5.0, 0.0);
	this->orbitLocation.radius = 20.0;
	this->orbitLocation.longitudeAngle = 0.0;
	this->orbitLocation.latitudeAngle = 0.0;
}

/*virtual*/ FollowCam::~FollowCam()
{
}

/*virtual*/ bool FollowCam::Setup()
{
	if (!this->camera || !this->subject)
		return false;

	this->MoveCameraOrbitBehindSubject();
	this->CalculateCameraPositionAndOrientation();

	this->freeCam = Game::Get()->SpawnEntity<FreeCam>();
	this->freeCam->SetCamera(this->camera);
	this->freeCam->SetEnabled(false);

	return true;
}

/*virtual*/ bool FollowCam::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ bool FollowCam::Tick(double deltaTime)
{
	Controller* controller = Game::Get()->GetController();
	if (controller->ButtonPressed(XINPUT_GAMEPAD_START))
	{
		this->freeCam->SetEnabled(!this->freeCam->IsEnabled());
	}

	if (!this->freeCam->IsEnabled())
	{
		if (controller->ButtonPressed(XINPUT_GAMEPAD_LEFT_SHOULDER))
			this->MoveCameraOrbitBehindSubject();

		Vector2 rightStick;
		controller->GetAnalogJoyStick(Controller::Side::RIGHT, rightStick.x, rightStick.y);

		double longitudeAngleDelta = this->followParams.maxRotationRate * deltaTime * rightStick.x;
		double latitudeAngleDelta = this->followParams.maxRotationRate * deltaTime * -rightStick.y;

		this->orbitLocation.longitudeAngle += longitudeAngleDelta;
		this->orbitLocation.latitudeAngle += latitudeAngleDelta;

		this->CalculateCameraPositionAndOrientation();
	}

	return true;
}

void FollowCam::MoveCameraOrbitBehindSubject()
{
	Transform subjectObjectToWorld;
	this->subject->GetTransform(subjectObjectToWorld);

	Vector3 xAxis, yAxis, zAxis;
	subjectObjectToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

	Vector3 upVector(0.0, 1.0, 0.0);
	Vector3 behindVector = zAxis.RejectedFrom(upVector).Normalized();
	SphericalCoords coords;
	coords.SetFromVector(behindVector);

	this->orbitLocation.longitudeAngle = coords.longitudeAngle;
}

void FollowCam::CalculateCameraPositionAndOrientation()
{
	Transform subjectObjectToWorld;
	this->subject->GetTransform(subjectObjectToWorld);

	Vector3 worldSpaceFocalPoint = subjectObjectToWorld.TransformPoint(this->followParams.objectSpaceFocalPoint);

	Vector3 cameraOffset = this->orbitLocation.GetToVector();

	Transform cameraToWorld;
	cameraToWorld.translation = worldSpaceFocalPoint + cameraOffset;

	Vector3 xAxis, yAxis, zAxis;
	Vector3 upAxis(0.0, 1.0, 0.0);

	zAxis = cameraOffset.Normalized();
	yAxis = upAxis.RejectedFrom(zAxis);
	xAxis = yAxis.Cross(zAxis);

	cameraToWorld.matrix.SetColumnVectors(xAxis, yAxis, zAxis);

	this->camera->SetCameraToWorldTransform(cameraToWorld);
}