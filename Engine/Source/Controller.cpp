#include "Controller.h"
#include <math.h>

using namespace Imzadi;

Controller::Controller(DWORD userIndex)
{
	this->userIndex = userIndex;
	::ZeroMemory(this->stateBuffer, sizeof(this->stateBuffer));
	this->stateIndex = 0;
	this->consumedButtonPresses = 0;
	this->consumedButtonReleases = 0;
}

/*virtual*/ Controller::~Controller()
{
}

void Controller::Update()
{
	DWORD result = XInputGetState(this->userIndex, &this->stateBuffer[this->stateIndex]);
	if (result == ERROR_SUCCESS)
		this->stateIndex = 1 - this->stateIndex;

	this->consumedButtonPresses = 0;
	this->consumedButtonReleases = 0;
}

const XINPUT_STATE* Controller::GetCurrentState()
{
	return &this->stateBuffer[1 - this->stateIndex];
}

const XINPUT_STATE* Controller::GetPreviousState()
{
	return &this->stateBuffer[this->stateIndex];
}

bool Controller::ButtonPressed(DWORD buttonFlag, bool consume /*= false*/)
{
	DWORD currentButtonFlags = 0, previousButtonFlags = 0;
	this->GetCurrentAndPreviousButtonFlags(currentButtonFlags, previousButtonFlags);
	bool pressed = ((currentButtonFlags & buttonFlag) != 0 && (previousButtonFlags & buttonFlag) == 0 && (this->consumedButtonPresses & buttonFlag) == 0);
	if (consume)
		this->consumedButtonPresses |= buttonFlag;
	return pressed;
}

bool Controller::ButtonReleased(DWORD buttonFlag, bool consume /*= false*/)
{
	DWORD currentButtonFlags = 0, previousButtonFlags = 0;
	this->GetCurrentAndPreviousButtonFlags(currentButtonFlags, previousButtonFlags);
	bool released = ((currentButtonFlags & buttonFlag) != 0 && (previousButtonFlags & buttonFlag) == 0 && (this->consumedButtonReleases & buttonFlag) == 0);
	if (consume)
		this->consumedButtonReleases |= buttonFlag;
	return released;
}

void Controller::GetCurrentAndPreviousButtonFlags(DWORD& currentButtonFlags, DWORD& previousButtonFlags)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();
	const XINPUT_STATE* previousState = this->GetPreviousState();

	currentButtonFlags = currentState->Gamepad.wButtons;
	previousButtonFlags = previousState->Gamepad.wButtons;
}

bool Controller::ButtonDown(DWORD buttonFlag)
{
	return((this->GetCurrentState()->Gamepad.wButtons & buttonFlag) != 0);
}

bool Controller::ButtonUp(DWORD buttonFlag)
{
	return((this->GetCurrentState()->Gamepad.wButtons & buttonFlag) == 0);
}

void Controller::GetAnalogJoyStick(Side side, double& x, double& y)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();

	int thumbX = 0, thumbY = 0;
	int deadZone = 0;
	
	switch (side)
	{
	case Side::LEFT:
		thumbX = currentState->Gamepad.sThumbLX;
		thumbY = currentState->Gamepad.sThumbLY;
		deadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		break;
	case Side::RIGHT:
		thumbX = currentState->Gamepad.sThumbRX;
		thumbY = currentState->Gamepad.sThumbRY;
		deadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		break;
	}
	
	if (::abs(thumbX) < deadZone)
		thumbX = 0;
	if (::abs(thumbY) < deadZone)
		thumbY = 0;

	x = double(thumbX) / 32768.0;
	y = double(thumbY) / 32768.0;
}

double Controller::GetTrigger(Side side)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();

	BYTE trigger = 0;

	switch (side)
	{
	case Side::LEFT:
		trigger = currentState->Gamepad.bLeftTrigger;
		break;
	case Side::RIGHT:
		trigger = currentState->Gamepad.bRightTrigger;
		break;
	}
	
	if (trigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		return 0.0;

	return double(trigger) / 255.0;
}