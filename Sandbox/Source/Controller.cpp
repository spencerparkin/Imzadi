#include "Controller.h"

using namespace Collision;

Controller::Controller()
{
	::ZeroMemory(this->stateBuffer, sizeof(this->stateBuffer));
	this->stateIndex = 0;
}

/*virtual*/ Controller::~Controller()
{
}

void Controller::Update()
{
	DWORD result = XInputGetState(0, &this->stateBuffer[this->stateIndex]);
	if (result == ERROR_SUCCESS)
		this->stateIndex = 1 - this->stateIndex;
}

const XINPUT_STATE* Controller::GetCurrentState()
{
	return &this->stateBuffer[1 - this->stateIndex];
}

Vector3 Controller::GetAnalogJoyStick(JoyStick joyStick)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();

	int thumbX = 0, thumbY = 0;
	int deadZone = 0;
	
	switch (joyStick)
	{
	case JoyStick::LEFT:
		thumbX = currentState->Gamepad.sThumbLX;
		thumbY = currentState->Gamepad.sThumbLY;
		deadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		break;
	case JoyStick::RIGHT:
		thumbX = currentState->Gamepad.sThumbRX;
		thumbY = currentState->Gamepad.sThumbRY;
		deadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		break;
	}
	
	if (abs(thumbX) < deadZone)
		thumbX = 0;
	if (abs(thumbY) < deadZone)
		thumbY = 0;

	Vector3 stickVector;

	stickVector.x = double(thumbX) / 32768.0;
	stickVector.y = double(thumbY) / 32768.0;
	stickVector.z = 0.0;
	
	return stickVector;
}