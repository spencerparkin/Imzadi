#pragma once

#include "Input.h"

namespace Imzadi
{
	/**
	 * Provide input using DirectInput library.
	 */
	class IMZADI_API DInput : public Input
	{
	public:
		DInput(int playerNumber);
		virtual ~DInput();

		virtual bool Setup() override;
		virtual bool Shutdown() override;
		virtual void Update(double deltaTime) override;
		virtual Vector2 GetAnalogJoyStick(Button button) override;
		virtual double GetTrigger(Button button) override;
		virtual bool ButtonPressed(Button button, bool consume = false) override;
		virtual bool ButtonReleased(Button button, bool consume = false) override;
		virtual bool ButtonDown(Button button) override;
		virtual bool ButtonUp(Button button) override;
	};
}