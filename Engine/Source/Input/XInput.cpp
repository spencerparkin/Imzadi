#include "XInput.h"

using namespace Imzadi;

XInput::XInput(int playerNumber) : Input(playerNumber)
{
	::ZeroMemory(this->stateBuffer, sizeof(this->stateBuffer));
	this->stateIndex = 0;
	this->consumedButtonPresses = 0;
	this->consumedButtonReleases = 0;
}

/*virtual*/ XInput::~XInput()
{
}

/*virtual*/ bool XInput::Setup(HWND windowHandle)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(this->playerNumber, &state);
	return result == ERROR_SUCCESS;
}

/*virtual*/ bool XInput::Shutdown()
{
	return true;
}

/*virtual*/ void XInput::Update(double deltaTime)
{
	DWORD result = XInputGetState(this->playerNumber, &this->stateBuffer[this->stateIndex]);
	if (result == ERROR_SUCCESS)
		this->stateIndex = 1 - this->stateIndex;

	this->consumedButtonPresses = 0;
	this->consumedButtonReleases = 0;
}

const XINPUT_STATE* XInput::GetCurrentState()
{
	return &this->stateBuffer[1 - this->stateIndex];
}

const XINPUT_STATE* XInput::GetPreviousState()
{
	return &this->stateBuffer[this->stateIndex];
}

/*virtual*/ Vector2 XInput::GetAnalogJoyStick(Button button)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();

	int thumbX = 0, thumbY = 0;
	int deadZone = 0;

	switch (button)
	{
	case Button::L_JOY_STICK:
		thumbX = currentState->Gamepad.sThumbLX;
		thumbY = currentState->Gamepad.sThumbLY;
		deadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		break;
	case Button::R_JOY_STICK:
		thumbX = currentState->Gamepad.sThumbRX;
		thumbY = currentState->Gamepad.sThumbRY;
		deadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		break;
	}

	if (::abs(thumbX) < deadZone)
		thumbX = 0;
	if (::abs(thumbY) < deadZone)
		thumbY = 0;

	return Vector2(double(thumbX), double(thumbY)) / 32768.0;
}

/*virtual*/ double XInput::GetTrigger(Button button)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();

	BYTE trigger = 0;

	switch (button)
	{
	case Button::L_TRIGGER:
		trigger = currentState->Gamepad.bLeftTrigger;
		break;
	case Button::R_TRIGGER:
		trigger = currentState->Gamepad.bRightTrigger;
		break;
	}

	if (trigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		return 0.0;

	return double(trigger) / 255.0;
}

/*virtual*/ bool XInput::ButtonPressed(Button button, bool consume /*= false*/)
{
	DWORD buttonFlag = this->GetButtonFlagForButton(button);
	if (!buttonFlag)
		return false;
	DWORD currentButtonFlags = 0, previousButtonFlags = 0;
	this->GetCurrentAndPreviousButtonFlags(currentButtonFlags, previousButtonFlags);
	bool pressed = ((currentButtonFlags & buttonFlag) != 0 && (previousButtonFlags & buttonFlag) == 0 && (this->consumedButtonPresses & buttonFlag) == 0);
	if (consume)
		this->consumedButtonPresses |= buttonFlag;
	return pressed;
}

/*virtual*/ bool XInput::ButtonReleased(Button button, bool consume /*= false*/)
{
	DWORD buttonFlag = this->GetButtonFlagForButton(button);
	if (!buttonFlag)
		return false;
	DWORD currentButtonFlags = 0, previousButtonFlags = 0;
	this->GetCurrentAndPreviousButtonFlags(currentButtonFlags, previousButtonFlags);
	bool released = ((currentButtonFlags & buttonFlag) != 0 && (previousButtonFlags & buttonFlag) == 0 && (this->consumedButtonReleases & buttonFlag) == 0);
	if (consume)
		this->consumedButtonReleases |= buttonFlag;
	return released;
}

/*virtual*/ bool XInput::ButtonDown(Button button)
{
	DWORD buttonFlag = this->GetButtonFlagForButton(button);
	if (!buttonFlag)
		return false;
	return ((this->GetCurrentState()->Gamepad.wButtons & buttonFlag) != 0);
}

/*virtual*/ bool XInput::ButtonUp(Button button)
{
	DWORD buttonFlag = this->GetButtonFlagForButton(button);
	if (!buttonFlag)
		return false;
	return ((this->GetCurrentState()->Gamepad.wButtons & buttonFlag) == 0);
}

DWORD XInput::GetButtonFlagForButton(Button button)
{
	switch (button)
	{
	case Button::A_BUTTON:
		return XINPUT_GAMEPAD_A;
	case Button::B_BUTTON:
		return XINPUT_GAMEPAD_B;
	case Button::X_BUTTON:
		return XINPUT_GAMEPAD_X;
	case Button::Y_BUTTON:
		return XINPUT_GAMEPAD_Y;
	case Button::DPAD_UP:
		return XINPUT_GAMEPAD_DPAD_UP;
	case Button::DPAD_DOWN:
		return XINPUT_GAMEPAD_DPAD_DOWN;
	case Button::DPAD_LEFT:
		return XINPUT_GAMEPAD_DPAD_LEFT;
	case Button::DPAD_RIGHT:
		return XINPUT_GAMEPAD_DPAD_RIGHT;
	case Button::L_SHOULDER:
		return XINPUT_GAMEPAD_LEFT_SHOULDER;
	case Button::R_SHOULDER:
		return XINPUT_GAMEPAD_RIGHT_SHOULDER;
	case Button::L_JOY_STICK:
		return XINPUT_GAMEPAD_LEFT_THUMB;
	case Button::R_JOY_STICK:
		return XINPUT_GAMEPAD_RIGHT_THUMB;
	case Button::START:
		return XINPUT_GAMEPAD_START;
	case Button::BACK:
		return XINPUT_GAMEPAD_BACK;
	}

	return 0;
}

void XInput::GetCurrentAndPreviousButtonFlags(DWORD& currentButtonFlags, DWORD& previousButtonFlags)
{
	const XINPUT_STATE* currentState = this->GetCurrentState();
	const XINPUT_STATE* previousState = this->GetPreviousState();

	currentButtonFlags = currentState->Gamepad.wButtons;
	previousButtonFlags = previousState->Gamepad.wButtons;
}