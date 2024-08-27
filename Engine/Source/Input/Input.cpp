#include "Input.h"

using namespace Imzadi;

Input::Input(int playerNumber)
{
	this->playerNumber = playerNumber;
}

/*virtual*/ Input::~Input()
{
}

/*virtual*/ void Input::HandleInputMessage(WPARAM wparam, LPARAM lParam)
{
}