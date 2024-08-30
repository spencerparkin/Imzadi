#pragma once

#include "Input.h"
#include <Xinput.h>

namespace Imzadi
{
	/**
	 * Provide input using the XInput library.
	 */
	class IMZADI_API XInput : public Input
	{
	public:
		XInput(int playerNumber);
		virtual ~XInput();

		virtual bool Setup(HWND windowHandle) override;
		virtual bool Shutdown() override;
		virtual void Update(double deltaTime) override;
		virtual Vector2 GetAnalogJoyStick(Button button) override;
		virtual double GetTrigger(Button button) override;
		virtual bool ButtonPressed(Button button, bool consume = false) override;
		virtual bool ButtonReleased(Button button, bool consume = false) override;
		virtual bool ButtonDown(Button button) override;
		virtual bool ButtonUp(Button button) override;

	private:
		const XINPUT_STATE* GetCurrentState();
		const XINPUT_STATE* GetPreviousState();

		void GetCurrentAndPreviousButtonFlags(DWORD& currentButtonFlags, DWORD& previousButtonFlags);

		DWORD GetButtonFlagForButton(Button button);

		XINPUT_STATE stateBuffer[2];
		int stateIndex;
		WORD consumedButtonPresses;
		WORD consumedButtonReleases;
	};
}