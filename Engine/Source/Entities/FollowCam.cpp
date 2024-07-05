#include "FollowCam.h"
#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"
#include "Game.h"
#include "FreeCam.h"
#include "Math/Angle.h"

using namespace Imzadi;

// TODO: Extra challenge: Adjust the radius of our camera's orbit so that no
//       collision object obstructs our view of the subject.  But don't really
//       change the orbit, just the effective orbit until the camera can
//       swivel out of the way.
FollowCam::FollowCam()
{
	this->followParams.maxRotationRate = M_PI / 1.5;
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

	this->MoveCameraOrbitBehindSubject(true);
	this->CalculateCameraPositionAndOrientation();

	this->freeCam = Game::Get()->SpawnEntity<FreeCam>();
	this->freeCam->SetCamera(this->camera);

	return true;
}

/*virtual*/ bool FollowCam::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ bool FollowCam::Tick(TickPass tickPass, double deltaTime)
{
	if (tickPass != TickPass::PARALLEL_TICK)
		return true;

	Controller* controller = Game::Get()->GetController("Hero");
	if (controller)
	{
		if (controller->ButtonPressed(XINPUT_GAMEPAD_START, true))
			this->freeCam->SetEnabled(true);

		if (controller->ButtonPressed(XINPUT_GAMEPAD_LEFT_SHOULDER))
			this->MoveCameraOrbitBehindSubject(false);

		Vector2 rightStick;
		controller->GetAnalogJoyStick(Controller::Side::RIGHT, rightStick.x, rightStick.y);

		double longitudeAngleDelta = this->followParams.maxRotationRate * deltaTime * rightStick.x;
		double latitudeAngleDelta = this->followParams.maxRotationRate * deltaTime * -rightStick.y;

		this->targetOrbitLocation.longitudeAngle += longitudeAngleDelta;
		this->targetOrbitLocation.latitudeAngle += latitudeAngleDelta;

		this->orbitLocation.Lerp(this->orbitLocation, this->targetOrbitLocation, 0.3);

		this->CalculateCameraPositionAndOrientation();
	}

	return true;
}

void FollowCam::MoveCameraOrbitBehindSubject(bool immediate)
{
	Transform subjectObjectToWorld;
	this->subject->GetTransform(subjectObjectToWorld);

	Vector3 xAxis, yAxis, zAxis;
	subjectObjectToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

	Vector3 upVector(0.0, 1.0, 0.0);
	Vector3 behindVector = zAxis.RejectedFrom(upVector).Normalized();
	SphericalCoords coords;
	coords.SetFromVector(behindVector);

	coords.longitudeAngle = Angle::MakeClose(coords.longitudeAngle, this->orbitLocation.longitudeAngle);

	this->targetOrbitLocation.latitudeAngle = this->orbitLocation.latitudeAngle;
	this->targetOrbitLocation.longitudeAngle = coords.longitudeAngle;
	this->targetOrbitLocation.radius = this->orbitLocation.radius;

	if (immediate)
		this->orbitLocation = this->targetOrbitLocation;
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