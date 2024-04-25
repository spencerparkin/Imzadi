#pragma once

#include <wx/app.h>
#include <Windows.h>
#include <Xinput.h>
#include "Math/Vector3.h"

class Controller
{
public:
	Controller();
	virtual ~Controller();

	void Update();

	enum JoyStick
	{
		LEFT,
		RIGHT
	};

	Collision::Vector3 GetAnalogJoyStick(JoyStick joyStick);

private:
	const XINPUT_STATE* GetCurrentState();

	XINPUT_STATE stateBuffer[2];
	int stateIndex;
};