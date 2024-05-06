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

	enum Side
	{
		LEFT,
		RIGHT
	};

	Collision::Vector3 GetAnalogJoyStick(Side side);
	double GetTrigger(Side side);

	bool ButtonPressed(DWORD buttonFlag);
	bool ButtonReleased(DWORD buttonFlag);
	bool ButtonDown(DWORD buttonFlag);
	bool ButtonUp(DWORD buttonFlag);

private:
	const XINPUT_STATE* GetCurrentState();
	const XINPUT_STATE* GetPreviousState();

	void GetCurrentAndPreviousButtonFlags(DWORD& currentButtonFlags, DWORD& previousButtonFlags);

	XINPUT_STATE stateBuffer[2];
	int stateIndex;
};