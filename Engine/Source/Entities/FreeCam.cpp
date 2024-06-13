#include "FreeCam.h"
#include "Game.h"
#include "Math/Vector2.h"

using namespace Imzadi;

FreeCam::FreeCam()
{
	this->speed = Speed::MEDIUM;
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

void FreeCam::SetEnabled(bool enabled)
{
	if (enabled)
		Game::Get()->PushControllerUser("FreeCam");
	else
		Game::Get()->PopControllerUser();
}

void FreeCam::SetCamera(Camera* camera)
{
	this->camera.Set(camera);
}

/*virtual*/ bool FreeCam::Tick(TickPass tickPass, double deltaTime)
{
	if (tickPass != TickPass::MID_TICK)
		return true;

	Controller* controller = Game::Get()->GetController("FreeCam");
	if (!controller)
		return true;

	Transform cameraToWorld = camera->GetCameraToWorldTransform();

	Vector3 xAxis, yAxis, zAxis;
	cameraToWorld.matrix.GetColumnVectors(xAxis, yAxis, zAxis);
	
	if (controller->ButtonPressed(XINPUT_GAMEPAD_START, true))
		this->SetEnabled(false);

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

	if (controller->ButtonPressed(XINPUT_GAMEPAD_LEFT_SHOULDER))
	{
		switch (this->speed)
		{
		case Speed::SLOW:
			this->speed = Speed::MEDIUM;
			break;
		case Speed::MEDIUM:
			this->speed = Speed::FAST;
			break;
		case Speed::FAST:
			this->speed = Speed::SLOW;
			break;
		}
	}

	Vector2 leftStick;
	controller->GetAnalogJoyStick(Controller::Side::LEFT, leftStick.x, leftStick.y);

	Vector3 strafeDelta(0.0, 0.0, 0.0);

	switch (this->strafeMode)
	{
	case StrafeMode::XZ_PLANE:
		strafeDelta = (leftStick.x * xAxis - leftStick.y * zAxis) * this->GetStrafeSpeed() * deltaTime;
		break;
	case StrafeMode::XY_PLANE:
		strafeDelta = (leftStick.x * xAxis + leftStick.y * yAxis) * this->GetStrafeSpeed() * deltaTime;
		break;
	}

	cameraToWorld.translation += strafeDelta;

	Vector2 rightStick;
	controller->GetAnalogJoyStick(Controller::Side::RIGHT, rightStick.x, rightStick.y);

	double headingDelta = -rightStick.x * this->GetRotationRate() * deltaTime;
	double pitchDelta = rightStick.y * this->GetRotationRate() * deltaTime;

	Matrix3x3 headingChangeMatrix, pitchChangeMatrix;

	headingChangeMatrix.SetFromAxisAngle(Vector3(0.0, 1.0, 0.0), headingDelta);
	pitchChangeMatrix.SetFromAxisAngle(xAxis, pitchDelta);

	cameraToWorld.matrix = headingChangeMatrix * pitchChangeMatrix * cameraToWorld.matrix;

	xAxis = cameraToWorld.matrix.GetColumnVector(0);
	xAxis = xAxis.RejectedFrom(Vector3(0.0, 1.0, 0.0));
	cameraToWorld.matrix.SetColumnVector(0, xAxis);
	cameraToWorld.matrix = cameraToWorld.matrix.Orthonormalized(IMZADI_AXIS_FLAG_X);

	camera->SetCameraToWorldTransform(cameraToWorld);

	return true;
}

double FreeCam::GetStrafeSpeed()
{
	switch (this->speed)
	{
	case Speed::SLOW:
		return 5.0;
	case Speed::MEDIUM:
		return 100.0;
	case Speed::FAST:
		return 200.0;
	}

	return 0.0;
}

double FreeCam::GetRotationRate()
{
	switch (this->speed)
	{
	case Speed::SLOW:
		return M_PI / 8.0;
	case Speed::MEDIUM:
	case Speed::FAST:
		return M_PI / 2.0;
	}

	return 0.0;
}