#include "PCInput.h"

using namespace Imzadi;

PCInput::PCInput(int playerNumber) : Input(playerNumber)
{
}

/*virtual*/ PCInput::~PCInput()
{
}

/*virtual*/ bool PCInput::Setup()
{
	return false;
}

/*virtual*/ bool PCInput::Shutdown()
{
	return false;
}

/*virtual*/ void PCInput::Update(double deltaTime)
{
}

/*virtual*/ Vector2 PCInput::GetAnalogJoyStick(Button button)
{
	return Vector2(0.0, 0.0);
}

/*virtual*/ double PCInput::GetTrigger(Button button)
{
	return 0.0;
}

/*virtual*/ bool PCInput::ButtonPressed(Button button, bool consume /*= false*/)
{
	return false;
}

/*virtual*/ bool PCInput::ButtonReleased(Button button, bool consume /*= false*/)
{
	return false;
}

/*virtual*/ bool PCInput::ButtonDown(Button button)
{
	return false;
}

/*virtual*/ bool PCInput::ButtonUp(Button button)
{
	return false;
}