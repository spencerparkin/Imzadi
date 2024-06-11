#pragma once

#include "Defines.h"

#if !defined WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Xinput.h>

namespace Imzadi
{
	/**
	 * This is a simple wrapper around the XINPUT API.  An instance of this
	 * class represents a connected controller and can be used to query for
	 * the state of that controller each frame or tick of an application.
	 */
	class IMZADI_API Controller
	{
	public:
		/**
		 * Construct an instance of the Controller class.
		 *
		 * @param[in] userIndex This refers to one of four possibly connected controllers.  It should be an integer in [0,3].
		 */
		Controller(DWORD userIndex);

		/**
		 * Destruct this instance.  Do nothing.
		 */
		virtual ~Controller();

		/**
		 * This should be called once per farme or tick of the application before calling
		 * any other method to query for the state of the controller to get input.
		 */
		void Update();

		/**
		 * These are used to refer to different sides of the controller in API calls.
		 */
		enum Side
		{
			LEFT,
			RIGHT
		};

		/**
		 * Get the XY location of a thumb-stick on the controller.
		 *
		 * @param[in] side This lets you specify the left or right thumbstick.
		 * @param[out] x This is the x-axis location of the stick in [0,1].
		 * @param[out] y this is the y-axis location of the stick in [0,1].
		 */
		void GetAnalogJoyStick(Side side, double& x, double& y);

		/**
		 * Get the value of a trigger on the controller.
		 *
		 * @param[in] side This lets you specify the left or right trigger.
		 * @return The trigger value is returned in [0,1].  Zero is not triggered at all.  One is fully triggered.
		 */
		double GetTrigger(Side side);

		/**
		 * Return true if and only if a button was pressed this frame.
		 *
		 * @param[in] buttonFlag This is an OR-ing of XINPUT_BUTTON_* flags.
		 * @param[in] consume If true, a subsequent call to ButtonPressed with the same flag before the next frame will indicate that the button was not pressed.
		 */
		bool ButtonPressed(DWORD buttonFlag, bool consume = false);

		/**
		 * Return true if and only if a button was released this frame.
		 *
		 * @param[in] buttonFlag This is an OR-ing of the XINPUT_BUTTON_* flags.
		 * @param[in] consume If true, a subsequent call to ButtonReleased with the same flag before the next frame will indicate that the button was not released.
		 */
		bool ButtonReleased(DWORD buttonFlag, bool consume = false);

		/**
		 * Return true if and only if a button is down this frame.
		 *
		 * @param[in] buttonFlag This is an OR-ing of the XINPUT_BUTTON_* flags.
		 */
		bool ButtonDown(DWORD buttonFlag);

		/**
		 * Return true if and only if a button is up this frame.
		 *
		 * @param[in] buttonFlag This is an OR-ing of the XINPUT_BUTTON_* flags.
		 */
		bool ButtonUp(DWORD buttonFlag);

	private:
		const XINPUT_STATE* GetCurrentState();
		const XINPUT_STATE* GetPreviousState();

		void GetCurrentAndPreviousButtonFlags(DWORD& currentButtonFlags, DWORD& previousButtonFlags);

		XINPUT_STATE stateBuffer[2];
		int stateIndex;
		DWORD userIndex;
		WORD consumedButtonPresses;
		WORD consumedButtonReleases;
	};
}