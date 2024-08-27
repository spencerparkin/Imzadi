#include "DInput.h"

using namespace Imzadi;

DInput::DInput(int playerNumber) : Input(playerNumber)
{
}

/*virtual*/ DInput::~DInput()
{
}

/*virtual*/ bool DInput::Setup()
{
	return false;
}

/*virtual*/ bool DInput::Shutdown()
{
	return false;
}

/*virtual*/ void DInput::Update(double deltaTime)
{
}

/*virtual*/ Vector2 DInput::GetAnalogJoyStick(Button button)
{
	return Vector2(0.0, 0.0);
}

/*virtual*/ double DInput::GetTrigger(Button button)
{
	return 0.0;
}

/*virtual*/ bool DInput::ButtonPressed(Button button, bool consume /*= false*/)
{
	return false;
}

/*virtual*/ bool DInput::ButtonReleased(Button button, bool consume /*= false*/)
{
	return false;
}

/*virtual*/ bool DInput::ButtonDown(Button button)
{
	return false;
}

/*virtual*/ bool DInput::ButtonUp(Button button)
{
	return false;
}