#include "FreeCam.h"
#include "Game.h"
#include "Math/Vector2.h"

using namespace Collision;

FreeCam::FreeCam()
{
	this->enabled = false;
	this->rotationRate = M_PI / 2.0;
	this->strafeSpeed = 100.0;
	this->strafeMode = StrafeMode::XZ_PLANE;
}

/*virtual*/ FreeCam::~FreeCam()
{
}

/*virtual*/ bool FreeCam::Setup()
{
	if (!this->camera)
		return false;

	return true;
}

/*virtual*/ bool FreeCam::Shutdown(bool gameShuttingDown)
{
	return true;
}

/*virtual*/ bool FreeCam::Tick(double deltaTime)
{
	if (!this->enabled)
		return true;

	Transform cameraToWorld = camera->GetCameraToWorldTransform();

	Vector3 xAxis, yAxis, zAxis;
	cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);

	Controller* controller = Game::Get()->GetController();
	
	if (controller->ButtonPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER))
	{
		switch (this->strafeMode)
		{
		case StrafeMode::XZ_PLANE:
			this->strafeMode = StrafeMode::XY_PLANE;
			break;
		case StrafeMode::XY_PLANE:
			this->strafeMode = StrafeMode::XZ_PLANE;
			break;
		}
	}

	Vector2 leftStick;
	controller->GetAnalogJoyStick(Controller::Side::LEFT, leftStick.x, leftStick.y);

	Vector3 strafeDelta(0.0, 0.0, 0.0);

	switch (this->strafeMode)
	{
	case StrafeMode::XZ_PLANE:
		strafeDelta = (leftStick.x * xAxis - leftStick.y * zAxis) * this->strafeSpeed * deltaTime;
		break;
	case StrafeMode::XY_PLANE:
		strafeDelta = (leftStick.x * xAxis + leftStick.y * yAxis) * this->strafeSpeed * deltaTime;
		break;
	}

	cameraToWorld.translation += strafeDelta;

	Vector2 rightStick;
	controller->GetAnalogJoyStick(Controller::Side::RIGHT, rightStick.x, rightStick.y);

	double headingDelta = -rightStick.x * this->rotationRate * deltaTime;
	double pitchDelta = rightStick.y * this->rotationRate * deltaTime;

	Matrix3x3 headingChangeMatrix, pitchChangeMatrix;

	headingChangeMatrix.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), headingDelta);
	pitchChangeMatrix.SetFromAxisAngle(xAxis, pitchDelta);

	cameraToWorld.matrix = headingChangeMatrix * pitchChangeMatrix * cameraToWorld.matrix;

	xAxis = cameraToWorld.matrix.GetColumnVector(0);
	xAxis = xAxis.RejectedFrom(Vector3(0.0, 1.0, 0.0));
	cameraToWorld.matrix.SetColumnVector(0, xAxis);
	cameraToWorld.matrix = cameraToWorld.matrix.Orthonormalized(COLL_SYS_AXIS_FLAG_X);

	camera->SetCameraToWorldTransform(cameraToWorld);

	return true;
}