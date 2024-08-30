#pragma once

#include "Defines.h"
#include "Math/Vector2.h"
#if !defined WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace Imzadi
{
	/**
	 * This is a set of platform-independent button defines.  It is based on
	 * a typical X-box controller, but there are usually analogs to these defines
	 * on most controller types.  For keyboard/mouse, these will be mapped to
	 * some buttons on a keyboard and a joy-stick might be controlled by a mouse,
	 * or just use arrow-keys.
	 */
	enum Button
	{
		X_BUTTON,
		Y_BUTTON,
		A_BUTTON,
		B_BUTTON,
		DPAD_UP,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT,
		L_JOY_STICK,
		R_JOY_STICK,
		L_SHOULDER,
		R_SHOULDER,
		L_TRIGGER,
		R_TRIGGER,
		BACK,
		START
	};

	/**
	 * This is the base class for any type of input source.  It represents
	 * a single input source (e.g., a controller or keyboard/mouse combo.)
	 */
	class IMZADI_API Input
	{
	public:
		Input(int playerNumber);
		virtual ~Input();

		/**
		 * This is where the input class derivative should try to detect and possibly
		 * connect to the a specific kind of input device, if present.
		 */
		virtual bool Setup(HWND windowHandle) = 0;

		/**
		 * Any clean-up should happen here.
		 */
		virtual bool Shutdown() = 0;

		/**
		 * This is called by the input system class each tick.
		 */
		virtual void Update(double deltaTime) = 0;

		/**
		 * Optionally override this if applicable.  This is where you can handle the WM_INPUT message.
		 */
		virtual void HandleInputMessage(WPARAM wparam, LPARAM lParam);

		/**
		 * Get the XY location of a thumb-stick on a controller.
		 *
		 * @param[in] button This lets you specify the left or right thumbstick.  If the given button doesn't represent a joystick, (0,0) should be returned.
		 * @return The position of the thumbstick should be returned in the range ([0,1],[0,1]).
		 */
		virtual Vector2 GetAnalogJoyStick(Button button) = 0;
		
		/**
		 * Get the value of a trigger on a controller.
		 *
		 * @param[in] button This lets you specify the left or right trigger.  If the given button doesn't represent a trigger, 0 should be returned.
		 * @return The trigger value is returned in [0,1].  Zero is not triggered at all.  One is fully triggered.
		 */
		virtual double GetTrigger(Button button) = 0;

		/**
		 * Return true if and only if a button was pressed this frame.
		 *
		 * @param[in] button This is the button in question.
		 * @param[in] consume If true, a subsequent call to ButtonPressed with the same button before the next frame will indicate that the button was not pressed.
		 */
		virtual bool ButtonPressed(Button button, bool consume = false) = 0;

		/**
		 * Return true if and only if a button was released this frame.
		 *
		 * @param[in] button This is the button in question.
		 * @param[in] consume If true, a subsequent call to ButtonReleased with the same flag before the next frame will indicate that the button was not released.
		 */
		virtual bool ButtonReleased(Button button, bool consume = false) = 0;

		/**
		 * Return true if and only if a button is down this frame.
		 *
		 * @param[in] button This is the button in question.
		 */
		virtual bool ButtonDown(Button button) = 0;

		/**
		 * Return true if and only if a button is up this frame.
		 *
		 * @param[in] button This is the button in question.
		 */
		virtual bool ButtonUp(Button button) = 0;

	protected:
		int playerNumber;
	};
}